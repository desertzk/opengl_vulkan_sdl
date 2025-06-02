#ifndef MODEL_H
#define MODEL_H

#include <glad/glad.h> 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <gli/gli.hpp>
#include <gli/gl.hpp>

#include "mesh.h"
#include "shader.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
using namespace std;

unsigned int TextureFromFile(const char *path, const string &directory, bool gamma = false);
unsigned int TextureFromMemory(unsigned char* buffer, size_t bufSize);
unsigned int TextureFromRawRGBA(unsigned char* pixels,
    unsigned int width,
    unsigned int height);




// Return 0 on failure, or the GL texture ID on success.
unsigned int LoadDDSFromFile(const std::string& fullPath)
{
    // 1) Use gli::load to read the file into a gli::texture.
    gli::texture Texture = gli::load(fullPath);
    if (Texture.empty()) {
        std::cerr << "[LoadDDSFromFile] Failed to load texture: " << fullPath << "\n";
        return 0;
    }

    // 2) Create an OpenGL texture name and bind it.
    GLenum Target = 0;
    {
        // gli::texture::target_type is an enum like gli::TARGET_2D, gli::TARGET_CUBE etc.
        // We must translate that to GL_TEXTURE_2D, GL_TEXTURE_CUBE_MAP, etc.
        gli::gl GLTranslator(gli::gl::PROFILE_GL33);
        Target = GLTranslator.translate(Texture.target());
    }

    GLuint TexID = 0;
    glGenTextures(1, &TexID);
    glBindTexture(Target, TexID);

    // 3) Now upload each mip‐level. gli may provide compressed (DXT1/5, BCn, etc.) or uncompressed data.
    {
        // Create a translator that maps gli formats/swizzles → OpenGL formats/types.
        // PROFILE_GL33 covers all desktop GL >= 3.3. If you need ES2/ES3, pick the correct profile.
        gli::gl GLTranslator(gli::gl::PROFILE_GL33);

        // Query the internal format (GLenum) and the pixel format/type (GLenum) from gli
        gli::gl::format Format = GLTranslator.translate(Texture.format(), Texture.swizzles());

        for (std::size_t Level = 0; Level < Texture.levels(); ++Level)
        {
            gli::texture::extent_type Extent = Texture.extent(Level);
            void* Data = Texture.data(0, 0, Level);

            if (gli::is_compressed(Texture.format()))
            {
                // For compressed data (DXT1/BC1, BC3, etc.)
                GLsizei BlockSize = static_cast<GLsizei>(Texture.size(Level));
                glCompressedTexImage2D(
                    Target,
                    static_cast<GLint>(Level),
                    Format.Internal,                     // e.g. GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
                    static_cast<GLsizei>(Extent.x),
                    static_cast<GLsizei>(Extent.y),
                    0,
                    BlockSize,
                    Data
                );
            }
            else
            {
                // For uncompressed formats (e.g. A8, RGBA8, RGB8, etc.)
                glTexImage2D(
                    Target,
                    static_cast<GLint>(Level),
                    Format.Internal,                     // e.g. GL_RGBA8
                    static_cast<GLsizei>(Extent.x),
                    static_cast<GLsizei>(Extent.y),
                    0,
                    Format.External,                     // e.g. GL_RGBA
                    Format.Type,                         // e.g. GL_UNSIGNED_BYTE
                    Data
                );
            }
        }
    }

    // 4) Set up standard filtering/wrapping.
    glTexParameteri(Target, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(Target, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // If this was a cubemap, also set R‐wrap. (Optional; most toolchains prefer CLAMP_TO_EDGE.)
    if (Target == GL_TEXTURE_CUBE_MAP) {
        glTexParameteri(Target, GL_TEXTURE_WRAP_R, GL_REPEAT);
    }

    glTexParameteri(Target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(Target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // 5) If the DDS file didn’t already provide all mip levels, generate missing ones.
    //    (Many DDS files—especially those from DXT exporters—embed every mip level automatically.)
    if (Texture.levels() == 1) {
        glGenerateMipmap(Target);
    }

    // 6) Unbind and return the GL texture ID.
    glBindTexture(Target, 0);
    return TexID;
}



class Model 
{
public:
    // model data 
    vector<Texture> textures_loaded;	// stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.
    vector<Mesh>    meshes;
    string directory;
    bool gammaCorrection;

    // constructor, expects a filepath to a 3D model.
    Model(string const &path, bool gamma = false) : gammaCorrection(gamma)
    {
        loadModel(path);
    }

    // draws the model, and thus all its meshes
    void Draw(Shader &shader)
    {
        for(unsigned int i = 0; i < meshes.size(); i++)
            meshes[i].Draw(shader);
    }
    
private:
    // loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
    void loadModel(string const &path)
    {
        // read file via ASSIMP
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
        // check for errors
        if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
        {
            cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
            return;
        }
        // retrieve the directory path of the filepath
        directory = path.substr(0, path.find_last_of('/'));

        // process ASSIMP's root node recursively
        processNode(scene->mRootNode, scene);
    }

    // processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
    void processNode(aiNode *node, const aiScene *scene)
    {
        // process each mesh located at the current node
        for(unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            // the node object only contains indices to index the actual objects in the scene. 
            // the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh, scene));
        }
        // after we've processed all of the meshes (if any) we then recursively process each of the children nodes
        for(unsigned int i = 0; i < node->mNumChildren; i++)
        {
            processNode(node->mChildren[i], scene);
        }

    }

    Mesh processMesh(aiMesh *mesh, const aiScene *scene)
    {
        // data to fill
        vector<Vertex> vertices;
        vector<unsigned int> indices;
        vector<Texture> textures;

        // walk through each of the mesh's vertices
        for(unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex;
            glm::vec3 vector; // we declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
            // positions
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.Position = vector;
            // normals
            if (mesh->HasNormals())
            {
                vector.x = mesh->mNormals[i].x;
                vector.y = mesh->mNormals[i].y;
                vector.z = mesh->mNormals[i].z;
                vertex.Normal = vector;
            }
            // texture coordinates
            if(mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
            {
                glm::vec2 vec;
                // a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
                // use models where a vertex can have multiple texture coordinates so we always take the first set (0).
                vec.x = mesh->mTextureCoords[0][i].x; 
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.TexCoords = vec;
                // tangent
                vector.x = mesh->mTangents[i].x;
                vector.y = mesh->mTangents[i].y;
                vector.z = mesh->mTangents[i].z;
                vertex.Tangent = vector;
                // bitangent
                vector.x = mesh->mBitangents[i].x;
                vector.y = mesh->mBitangents[i].y;
                vector.z = mesh->mBitangents[i].z;
                vertex.Bitangent = vector;
            }
            else
                vertex.TexCoords = glm::vec2(0.0f, 0.0f);

            vertices.push_back(vertex);
        }
        // now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
        for(unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            // retrieve all indices of the face and store them in the indices vector
            for(unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);        
        }
        // process materials
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];    
        // we assume a convention for sampler names in the shaders. Each diffuse texture should be named
        // as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER. 
        // Same applies to other texture as the following list summarizes:
        // diffuse: texture_diffuseN
        // specular: texture_specularN
        // normal: texture_normalN

        // 1. diffuse maps
        vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse", scene);
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        // 2. specular maps
        vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular", scene);
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
        // 3. normal maps
        std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal", scene);
        textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
        // 4. height maps
        std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height", scene);
        textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
        
        // return a mesh object created from the extracted mesh data
        return Mesh(vertices, indices, textures);
    }

    // checks all material textures of a given type and loads the textures if they're not loaded yet.
    // the required info is returned as a Texture struct.
    vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, string typeName)
    {
        vector<Texture> textures;
        for(unsigned int i = 0; i < mat->GetTextureCount(type); i++)
        {
            aiString str;
            mat->GetTexture(type, i, &str);
            // check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
            bool skip = false;
            for(unsigned int j = 0; j < textures_loaded.size(); j++)
            {
                if(std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
                {
                    textures.push_back(textures_loaded[j]);
                    skip = true; // a texture with the same filepath has already been loaded, continue to next one. (optimization)
                    break;
                }
            }
            if(!skip)
            {   // if texture hasn't been loaded already, load it
                Texture texture;
                texture.id = TextureFromFile(str.C_Str(), this->directory);
                texture.type = typeName;
                texture.path = str.C_Str();
                textures.push_back(texture);
                textures_loaded.push_back(texture);  // store it as texture loaded for entire model, to ensure we won't unnecessary load duplicate textures.
            }
        }
        return textures;
    }


    vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type,
        const string& typeName, const aiScene* scene)
    {
        vector<Texture> textures;
        for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
            aiString str;
            mat->GetTexture(type, i, &str);

            // 1) Has this texture already been loaded (by exact path/embed index)? 
            bool skip = false;
            for (unsigned int j = 0; j < textures_loaded.size(); j++) {
                if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0) {
                    textures.push_back(textures_loaded[j]);
                    skip = true;
                    break;
                }
            }
            if (skip)
                continue;

            Texture texture;
            texture.type = typeName;
            texture.path = str.C_Str();

            // 2) Is this an embedded texture? Assimp uses a leading '*' for embedded.
            if (str.C_Str()[0] == '*') {
                // e.g. str == "*0"  �� index = 0
                unsigned int texIndex = atoi(str.C_Str() + 1);
                if (texIndex < scene->mNumTextures && scene->mTextures[texIndex]->mHeight == 0) {
                    // Uncompressed in-memory data:
                    aiTexture* aiTex = scene->mTextures[texIndex];
                    // aiTex->pcData is raw PNG/JPEG (aiTex->mWidth bytes)
                    texture.id = TextureFromMemory(
                        reinterpret_cast<unsigned char*>(aiTex->pcData),
                        aiTex->mWidth
                    );
                }
                else {
                    // Some FBX exporters embed as ��compressed�� (mHeight > 0):
                    aiTexture* aiTex = scene->mTextures[texIndex];
                    // aiTex->pcData is raw RGBA8888 (width=width, height=height)
                    texture.id = TextureFromRawRGBA(
                        reinterpret_cast<unsigned char*>(aiTex->pcData),
                        aiTex->mWidth,            // width in pixels
                        aiTex->mHeight            // height in pixels
                    );
                }
            }
            else {
                // 3) It��s an external file��just load from disk as before:
                texture.id = TextureFromFile(str.C_Str(), this->directory);
            }

            textures.push_back(texture);
            textures_loaded.push_back(texture);
        }

        return textures;
    }





};


// Example: decode a compressed buffer (PNG/JPEG) in RAM
unsigned int TextureFromMemory(unsigned char* buffer, size_t bufSize)
{
    int width, height, nrChannels;
    unsigned char* data = stbi_load_from_memory(buffer, bufSize,
        &width, &height,
        &nrChannels, 0);
    if (!data) {
        std::cerr << "Failed to load texture from memory\n";
        return 0;
    }
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    GLenum format = (nrChannels == 4 ? GL_RGBA : GL_RGB);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height,
        0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);
    return textureID;
}

// Example: upload raw RGBA8888 image already provided by Assimp
unsigned int TextureFromRawRGBA(unsigned char* pixels,
    unsigned int width,
    unsigned int height)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    // The data is already RGBA8
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height,
        0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    glGenerateMipmap(GL_TEXTURE_2D);
    return textureID;
}




unsigned int TextureFromFile(const char *path,
                             const std::string &directory,
                             bool gamma)
{
    // 1) build absolute filename exactly as before
    bool isAbsolute = false;
    if (path[0] == '/' ||
       (std::strlen(path) >= 2 && std::isalpha(path[0]) && path[1] == ':'))
    {
        isAbsolute = true;
    }

    std::string filename;
    if (isAbsolute) {
        filename = std::string(path);
    } else {
        filename = directory + "/" + std::string(path);
    }

    // 2) check extension:
    auto ext = std::string(path);
    // find last dot, lower-case it
    size_t dot = ext.find_last_of('.');
    if (dot != std::string::npos) {
        ext = ext.substr(dot + 1);
        std::transform(ext.begin(), ext.end(), ext.begin(),
                       [](unsigned char c){ return std::tolower(c); });
    } else {
        ext = ""; // no extension found
    }

    // If it’s a DDS, hand it off to a DDS loader
    if (ext == "dds") {
        unsigned int textureID = LoadDDSFromFile(filename);
        if (textureID == 0) {
            std::cout << "DDS load failed at path: " << filename << std::endl;
        }
        return textureID;
    }

    // 3) Otherwise, use stb_image as before:
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;
        else
            format = GL_RGB; // fallback

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height,
                     0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << filename << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

#endif
