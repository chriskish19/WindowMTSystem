#include "WindowResources.hpp"

void WMTS::WindowResources::AddToThreadmp(const std::thread::id t_id,const HWND WindowHandle){
    std::lock_guard<std::mutex> local_lock(mThreadmp_mtx);
	mThread_mp.emplace(t_id,WindowHandle);
}

std::optional<HWND> WMTS::WindowResources::SearchThreadmp(const std::thread::id& t_id){
    std::lock_guard<std::mutex> local_lock(mThreadmp_mtx);
    auto found = mThread_mp.find(t_id);
    if(found != mThread_mp.end()){
        return found->second;
    }
    return std::nullopt;
}

void WMTS::WindowResources::RemoveFromThreadmp(auto position){
    std::lock_guard<std::mutex> local_lock(mThread_mp);
    mThread_mp.erase(position);
}

void WMTS::WindowResources::AddToWindowHandles(const HWND WindowHandle){
    std::lock_guard<std::mutex> local_lock(mWindowHandles_mtx);
    mWindowHandles.push_back(WindowHandle);
}

std::optional<HWND> WMTS::WindowResources::SearchWindowHandles(const HWND WindowHandle){
    std::lock_guard<std::mutex> local_lock(mWindowHandles_mtx);
    auto found = std::find(mWindowHandles.begin(),mWindowHandles.end(),WindowHandle);
    if(found != mWindowHandles.end()){
        return *found;
    }
    return std::nullopt;
}

void WMTS::WindowResources::RemoveFromWindowHandles(auto entry){
    std::lock_guard<std::mutex> local_lock(mWindowHandles_mtx);
    mWindowHandles.erase(entry);
}

HWND WMTS::WindowResources::GetWindowHandle(size_t index){
    std::lock_guard<std::mutex> local_lock(mWindowHandles_mtx);
    if(mWindowHandles.empty()) return nullptr;
    index = std::clamp(index, (size_t)0, mWindowHandles.size() - 1);
    return mWindowHandles[index];
}

void WMTS::WindowResources::RemoveFromWindowmp(auto entry){
    std::lock_guard<std::mutex> local_lock(mWindowmp_mtx);
    mWindow_mp.erase(entry);
}

void WMTS::WindowResources::AddToThreadpoolmp(std::thread::id t_id,std::thread* t_p){
    std::lock_guard<std::mutex> local_lock(mThreadpoolmp_mtx);
    mThread_pool_mp.emplace(t_id,t_p);
}

std::optional<std::thread*> WMTS::WindowResources::SearchThreadpoolmp(const std::thread::id t_id){
    std::lock_guard<std::mutex> local_lock(mThreadpoolmp_mtx);
    auto found = mThread_pool_mp.find(t_id);
    if(found != mThread_pool_mp.end()){
        return found->second;
    }
    return std::nullopt;
}

std::unordered_map<std::thread::id, std::thread*>::iterator WMTS::WindowResources::ItSearchThreadpoolmp(const std::thread::id t_id){
    std::lock_guard<std::mutex> local_lock(mThreadpoolmp_mtx);
    return mThread_pool_mp.find(t_id);
}

void WMTS::WindowResources::RemoveFromThreadpoolmp(auto entry){
    std::lock_guard<std::mutex> local_lock(mThreadpoolmp_mtx);
    mThread_pool_mp.erase(entry);
}

bool WMTS::WindowResources::GetThreadpoolmpEmptyState(){
    std::lock_guard<std::mutex> local_lock(mThreadpoolmp_mtx);
    return mThread_pool_mp.empty();
}

std::unordered_map<std::thread::id, std::thread*>::iterator WMTS::WindowResources::ItGetEndofThreadpoolmp(){
    std::lock_guard<std::mutex> local_lock(mThreadpoolmp_mtx);
    return mThread_pool_mp.end();
}

size_t WMTS::WindowResources::GetThreadpoolmpSize(){
    std::lock_guard<std::mutex> local_lock(mThreadpoolmp_mtx);
    return mThread_pool_mp.size();
}

void WMTS::WindowResources::Update(const std::thread::id t_id){
    // scoped thread lock
    // cant use the member functions here as that would result in a deadlock
    // also we want to block any member function calls to mThread_pool_mp during Update()
    // could use a recursive_mutex?
    {
        std::lock_guard<std::mutex> local_lock(mThreadpoolmp_mtx);
        auto found = mThread_pool_mp.find(t_id);
        if(found != mThread_pool_mp.end()){
            std::thread* t = found->second;
            if(t->joinable()){
                // detach the thread
                t->detach();

                // Now that the thread is detached, we can delete the std::thread object.
                delete t;

                // erase the entry
                mThread_pool_mp.erase(found);
            }
            else{
                return;
            }
        }
        else{
            return;
        }
    }



    HWND FoundWindowHandle;
    {
        std::lock_guard<std::mutex> local_lock(mThreadmp_mtx);
        auto found = mThread_mp.find(t_id);
        if(found != mThread_mp.end()){
            FoundWindowHandle = found->second;

            // erase the entry
            mThread_mp.erase(found);
        }
        else{
            return;
        }
    }



    {
        std::lock_guard<std::mutex> local_lock(mWindowHandles_mtx);
        // update mWindowHandles vector
        auto found = std::find(mWindowHandles.begin(), mWindowHandles.end(), FoundWindowHandle);
        if (found != mWindowHandles.end()) {
            // erase the entry
            mWindowHandles.erase(found);
        }
        else{
            return;
        }
    }


    {
        std::lock_guard<std::mutex> local_lock(mWindowmp_mtx);
        auto found = mWindow_mp.find(FoundWindowHandle);
        if (found != mWindow_mp.end()) {
            // erase the entry
            mWindow_mp.erase(found);
        }
        else{
            return;
        }
    }
}

void WMTS::WindowResources::AddToWindowmp(const HWND WindowHandle,const WindowDimensions size){
    std::lock_guard<std::mutex> local_lock(mWindowmp_mtx);
    mWindow_mp.emplace(WindowHandle,size);
}

std::optional<WMTS::WindowDimensions> WMTS::WindowResources::SearchWindowmp(const HWND WindowHandle){
    std::lock_guard<std::mutex> local_lock(mWindowmp_mtx);
    auto found = mWindow_mp.find(WindowHandle);
    if(found != mWindow_mp.end()){
        return found->second;
    }
    return std::nullopt;
}