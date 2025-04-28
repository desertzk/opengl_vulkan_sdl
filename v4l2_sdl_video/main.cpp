#include <SDL3/SDL.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include<string>
#include <stdexcept>

// Number of buffers for memory-mapped I/O
const int N_BUFFERS = 4;

struct Buffer {
    void* start;
    size_t length;
};

int main(int argc, char* argv[]) {
    const char* device = "/dev/video0";
    const char* outfile = "capture.yuv";
    int fd = open(device, O_RDWR);
    if (fd < 0) {
        perror("Opening video device");
        return EXIT_FAILURE;
    }

    // 1. Query capabilities
    struct v4l2_capability cap = {};
    if (ioctl(fd, VIDIOC_QUERYCAP, &cap) < 0) {
        perror("VIDIOC_QUERYCAP");
        return EXIT_FAILURE;
    }

    // 2. Set format: YUYV 640x480 @ 30fps
    struct v4l2_format fmt = {};
    fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width       = 640;
    fmt.fmt.pix.height      = 480;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
    fmt.fmt.pix.field       = V4L2_FIELD_ANY;
    if (ioctl(fd, VIDIOC_S_FMT, &fmt) < 0) {
        perror("VIDIOC_S_FMT");
        return EXIT_FAILURE;
    }

    // 3. Request buffers (MMAP)
    struct v4l2_requestbuffers req = {};
    req.count  = N_BUFFERS;
    req.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;
    if (ioctl(fd, VIDIOC_REQBUFS, &req) < 0) {
        perror("VIDIOC_REQBUFS");
        return EXIT_FAILURE;
    }

    // 4. Map buffers
    std::vector<Buffer> buffers(req.count);
    for (auto& buf : buffers) {
        struct v4l2_buffer vbuf = {};
        vbuf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        vbuf.memory = V4L2_MEMORY_MMAP;
        vbuf.index  = &buf - &buffers[0];
        if (ioctl(fd, VIDIOC_QUERYBUF, &vbuf) < 0) {
            perror("VIDIOC_QUERYBUF");
            return EXIT_FAILURE;
        }
        buf.length = vbuf.length;
        buf.start  = mmap(nullptr, buf.length,
                          PROT_READ | PROT_WRITE,
                          MAP_SHARED, fd, vbuf.m.offset);
        if (buf.start == MAP_FAILED) {
            perror("mmap");
            return EXIT_FAILURE;
        }
    }

    // 5. Queue buffers
    for (unsigned i = 0; i < buffers.size(); ++i) {
        struct v4l2_buffer vbuf = {};
        vbuf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        vbuf.memory = V4L2_MEMORY_MMAP;
        vbuf.index  = i;
        ioctl(fd, VIDIOC_QBUF, &vbuf);
    }

    // 6. Start streaming
    int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ioctl(fd, VIDIOC_STREAMON, &type);

    // Open output file
    FILE* out = std::fopen(outfile, "wb");
    if (!out) {
        perror("Opening output file");
        return EXIT_FAILURE;
    }

    // Initialize SDL3
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        throw std::runtime_error("SDL_Init failed: " + std::string(SDL_GetError()));
    }

    // Create window (width, height, flags)
    SDL_Window* win = SDL_CreateWindow(
        "V4L2 + SDL3 Capture",
        fmt.fmt.pix.width, fmt.fmt.pix.height,
        0                               // no flags
    );
    // Create renderer (name=nullptr lets SDL pick)
    SDL_Renderer* ren = SDL_CreateRenderer(win, nullptr);
    if (!ren) {
        SDL_Log("SDL_CreateRenderer Error: %s", SDL_GetError());
        return EXIT_FAILURE;
    }
    SDL_Texture* tex = SDL_CreateTexture(ren,
        SDL_PIXELFORMAT_YUY2, SDL_TEXTUREACCESS_STREAMING,
        fmt.fmt.pix.width, fmt.fmt.pix.height);

    // Main loop
    bool running = true;
    while (running) {
        // Handle SDL events
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) running = false;
        }

        // Dequeue buffer
        struct v4l2_buffer vbuf = {};
        vbuf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        vbuf.memory = V4L2_MEMORY_MMAP;
        ioctl(fd, VIDIOC_DQBUF, &vbuf);

        // Write raw YUYV to file
        fwrite(buffers[vbuf.index].start, 1, buffers[vbuf.index].length, out);

        // Update SDL texture and render
        SDL_UpdateTexture(tex, nullptr,
                          buffers[vbuf.index].start,
                          fmt.fmt.pix.width * 2);
        SDL_RenderClear(ren);
        SDL_RenderTexture(ren, tex, nullptr, nullptr);
        SDL_RenderPresent(ren);

        // Re-queue buffer
        ioctl(fd, VIDIOC_QBUF, &vbuf);
    }

    // Cleanup
    SDL_DestroyTexture(tex);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();

    std::fclose(out);
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ioctl(fd, VIDIOC_STREAMOFF, &type);
    for (auto& buf : buffers) munmap(buf.start, buf.length);
    close(fd);
    return EXIT_SUCCESS;
}
