#pragma once

#include <thread>
#include <mutex>
#include <condition_variable>

namespace thm {
    class ThreadManager {
        public:
            ThreadManager(vkr::VulkanRendering& v, ecs::EntityManager& e) : vkr{v}, em{e} {
                InitializeThreads();

                // Everything below is testing (except for functions)

                std::this_thread::sleep_for(std::chrono::seconds(1)); // NEED TO CHANGE THIS AT SOME POINT TO JUST BE SYNCED
                auto& entity = em.AddEntity();
                AddEntityToRenderer<vkr::TriangleRenderer2D>(em.entities.at(0));
                CopyToVulkanRenderingThread();
                {
                    std::lock_guard<std::mutex> lock(mtx);
                    render_pause = true;
                    cv.notify_all();
                }
                std::vector<Vertex2D> vertices = {{{0.5,0.5},{1,0,0,1},{0,0}},{{-0.5,0.5},{0,1,0,1},{0,0}},{{0,-0.5},{0,0,1,1},{0,0}}};
                auto mesh = vkr.GetMeshPool()->CreateMesh(vertices);
                entity.AddComponent<ecs::Mesh2D>(mesh);
                auto& entity2 = em.AddEntity();
                AddEntityToRenderer<vkr::TriangleRenderer2D>(em.entities.at(1));
                std::vector<Vertex2D> vertices2 = {{{0.2,0.2},{1,1,0,1},{0,0}},{{-0.2,0.2},{0,1,1,1},{0,0}},{{0,-0.2},{1,0,1,1},{0,0}}};
                auto mesh2 = vkr.GetMeshPool()->CreateMesh(vertices2);
                entity2.AddComponent<ecs::Mesh2D>(mesh2);
                {
                    std::lock_guard<std::mutex> lock(mtx);
                    render_pause = false;
                    cv.notify_all();
                }
                CopyToVulkanRenderingThread();

                vkrThread.join();
            }

            void InitializeThreads() {
                vkrThread = std::thread(&ThreadManager::VulkanRenderingThread, this);
            }

            void VulkanRenderingThread() {
                vkr.Init();
                RendererEntities[std::make_shared<vkr::TriangleRenderer3D>(vkr.GetDevice(), vkr.GetSwapchain())] = {};
                RendererEntities[std::make_shared<vkr::TriangleRenderer2D>(vkr.GetDevice(), vkr.GetSwapchain())] = {};
                while (!glfwWindowShouldClose(vkr.GetWindow().getWindow())) {
                    std::unique_lock<std::mutex> lock(mtx);
                    cv.wait(lock, [this]{ return !render_pause; });
                    vkr.Run();
                }
            }

            void CopyToVulkanRenderingThread() {
                {
                    std::lock_guard<std::mutex> lock(mtx);
                    render_pause = true;
                    cv.notify_all();
                }
                CopyRendererEntitiesToRenderers();
                {
                    std::lock_guard<std::mutex> lock(mtx);
                    render_pause = false;
                    cv.notify_all();
                }
            }

            void UpdateMeshPool() {

            }

            template <class T, typename = std::enable_if_t<std::is_base_of<vkr::Renderer, T>::value>>
            void AddEntityToRenderer(std::shared_ptr<ecs::Entity> entity) {
                for (auto& pair : RendererEntities) {
                    if (typeid(*pair.first) == typeid(T)) {
                        pair.second.push_back(entity);
                        break;
                    }
                }
            }

            void CopyRendererEntitiesToRenderers() {
                vkr.SetRendererEntities(RendererEntities);
            }
        private:
            vkr::VulkanRendering& vkr;
            ecs::EntityManager& em;

            std::unordered_map<std::shared_ptr<vkr::Renderer>, std::vector<std::shared_ptr<ecs::Entity>>> RendererEntities;

            std::thread vkrThread;
            std::thread emThread;

            bool finish = false;
            bool render_pause = false;

            std::mutex mtx;
            std::condition_variable cv;
    };
}