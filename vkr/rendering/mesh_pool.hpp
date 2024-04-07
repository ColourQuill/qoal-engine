#pragma once

namespace vkr {
    class Mesh {
        public:
            Mesh(std::shared_ptr<BufferManager> bm, std::vector<Vertex2D>& v) : vertices2D{v} {
                vertexBuffer = bm->CreateVertexBuffer(v);
            }
            Mesh(std::shared_ptr<BufferManager> bm, std::vector<Vertex3D>& v) : vertices3D{v} {
                vertexBuffer = bm->CreateVertexBuffer(v);
            }
            Mesh(std::shared_ptr<BufferManager> bm, std::vector<Vertex2D>& v, std::vector<uint32_t>& i) : vertices2D{v}, indices{i} {
                vertexBuffer = bm->CreateVertexBuffer(v);
            }
            Mesh(std::shared_ptr<BufferManager> bm, std::vector<Vertex3D>& v, std::vector<uint32_t>& i) : vertices3D{v}, indices{i} {
                vertexBuffer = bm->CreateVertexBuffer(v);
            }

            std::shared_ptr<Buffer> GetVertexBuffer() {
                return vertexBuffer;
            }
            std::shared_ptr<Buffer> GetIndexBuffer() {
                return indexBuffer;
            }
        private:
            std::vector<Vertex2D> vertices2D;
            std::vector<Vertex3D> vertices3D;
            std::vector<uint32_t> indices;

            std::shared_ptr<Buffer> vertexBuffer;
            std::shared_ptr<Buffer> indexBuffer;
    };

    class MeshPool {
        public:
            MeshPool(std::shared_ptr<BufferManager> bm) : bufferManager{bm} {

            }
            std::shared_ptr<Mesh> CreateMesh(std::vector<Vertex2D>& v) {
                return std::make_shared<Mesh>(bufferManager, v);
            }
            std::shared_ptr<Mesh> CreateMesh(std::vector<Vertex3D>& v) {
                return std::make_shared<Mesh>(bufferManager, v);
            }
            std::shared_ptr<Mesh> CreateMesh(std::vector<Vertex2D>& v, std::vector<uint32_t>& i) {
                return std::make_shared<Mesh>(bufferManager, v, i);
            }
            std::shared_ptr<Mesh> CreateMesh(std::vector<Vertex3D>& v, std::vector<uint32_t>& i) {
                return std::make_shared<Mesh>(bufferManager, v, i);
            }
            std::shared_ptr<Mesh> AddMeshToMeshPool(std::shared_ptr<Mesh> mesh) {
                mesh_pool.push_back(mesh);
                return mesh;
            }
        private:
            std::shared_ptr<BufferManager> bufferManager;

            std::vector<std::shared_ptr<Mesh>> mesh_pool;
    };
}