#include"object.hpp"


void Object::loadModel(const std::string& path)
{
    Assimp::Importer import;
    //aiProcess_Triangulate clip triangles
    //aiProcess_FlipUVs reverse y
    //aiProcess_GenNormals
    //aiProcess_SplitLargeMeshes split big mesh into small
    //aiProcess_OptimizeMeshes merge small meshes into a big mesh

    const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
        return;
    }
    //directory = path.substr(0, path.find_last_of('/'));

    processNode(scene->mRootNode, scene);
    createFinish = true;
}

void Object::processNode(aiNode* node, const aiScene* scene)
{
    // 处理节点所有的网格（如果有的话）
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(std::shared_ptr<MeshTriangle>(processMesh(mesh, scene)));
    }
    // 接下来对它的子节点重复这一过程
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene);
    }
}

MeshTriangle* Object::processMesh(aiMesh* mesh, const aiScene* scene)
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    for (unsigned i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;

        glm::vec3 vector;
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.Position = vector;

        if (mesh->HasNormals()) {
            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vertex.Normal = vector;
        }
        
        if (mesh->mTextureCoords[0])
        {
            glm::vec2 vec;
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.TexCoords = vec;
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
    // 处理索引
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }
    // 处理材质
    /*if (mesh->mMaterialIndex >= 0)
    {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        std::vector<Texture> diffuseMaps = loadMaterialTextures(material,
            aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        std::vector<Texture> specularMaps = loadMaterialTextures(material,
            aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
    }*/

    return new MeshTriangle(vertices, indices);
}

void Object::draw(int meshIndex) const&
{
    if (!createFinish)
        return;
    if (meshIndex < meshes.size() && meshIndex >= 0)
        meshes.at(meshIndex)->draw();
    else if (meshIndex == -1)
        for (auto& i : meshes)
            i->draw();
}

void Object::draw(Shader* s) const&
{
    if (!createFinish)
        return;
    for (auto& i : meshes)
        i->draw(s);
}

void Object::drawGBuffer() const&
{
    if (!createFinish)
        return;
    for (auto& i : meshes) {
        i->drawGBuffer();
        //return;
    }
        
}

void Object::drawInstance(unsigned size) const&
{
    for (auto& m : meshes)
        m->drawInstance(size);
}

std::shared_ptr<Object> CreateObject(const std::string& path) {
    return std::make_shared<Object>(path);
}

std::shared_ptr<Object> CreateObject() {
    return std::make_shared<Object>();
}

std::shared_ptr<Object> CopyObject(std::shared_ptr<Object> obj) {
    return std::shared_ptr<Object>(new Object(*obj.get()));
}
