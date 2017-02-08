/*
    App.h nachocpol@gmail.com
*/

#pragma once

class App
{
public:
    App();
    ~App();
    void Start();

private:
    virtual bool Init       () = 0;
    virtual void Update     () = 0;
    virtual void Render     () = 0;
    virtual void RenderUi   () = 0;
    virtual void Release    () = 0;

protected:
    bool mRunning;
    float mTime;
    float mDeltaTime;
    int mFps;
    const float kFpsUpdateTime = 0.15f;
    float mFpsCurUpdate;
};
