/* date = December 26th 2022 0:18 am */

#ifndef CAMERA_H
#define CAMERA_H

enum camera_mode
{
    CAMERA_FPS,
    CAMERA_ARC,
    CAMERA_WEIRD,
	
	CAMERA_COUNT,
};

char *camera_mode_label[CAMERA_COUNT] = 
{
	"fps",
	"arc",
	"weird",
};

struct game_camera
{
    matrix wvp;
    matrix world;
    matrix view;
    matrix projection3d;
    matrix projection2d;
    matrix invProjection2d;
	
    v3 position;
    v3 target;
    
    v3 up;
    r32 fov;
    r32 ratio;
    
    r32 yaw   = 0.0f;
    r32 pitch = 0.0f;
    
    camera_mode mode;
    
    r32 radius; // TPS mode
};

inline void
CameraLookAtLH(matrix *Output, v3 pos, v3 lookAt, v3 up)
{
    // REFERENCE: 
    // https://docs.microsoft.com/en-us/windows/win32/direct3d9/d3dxmatrixlookatlh
    v3 result = lookAt - pos;
    v3 zAxis  = Normalize(&result);
    
    result = up * zAxis;
    v3 xAxis  = Normalize(&result);
    
    result = zAxis * xAxis;
    v3 yAxis  = Normalize(&result);
    
    r32 tx = -Dot(&xAxis, &pos);
    r32 ty = -Dot(&yAxis, &pos);
    r32 tz = -Dot(&zAxis, &pos);
    
    Output->E[0] = V4( xAxis.x, yAxis.x, zAxis.x, 0 );
    Output->E[1] = V4( xAxis.y, yAxis.y, zAxis.y, 0 );
    Output->E[2] = V4( xAxis.z, yAxis.z, zAxis.z, 0 );
    Output->E[3] = V4(      tx,      ty,      tz, 1.0f );
}

// NOTE: for tps 
inline void
BasisTransform(matrix *Output, v3 *Position, v3 *LookAt)
{
    v3 Pos = *Position;
    v3 Look = *LookAt;
    v3 Up = V3(0.0f, 1.0f, 0.0f);
    
    v3 Result = Look - Pos;
    Result.y = 0;
    v3 ZAxis  = Normalize(&Result);
    
    Result = Up * ZAxis;
    v3 XAxis  = Normalize(&Result);
    
    Output->E[0] = V4( XAxis.x, 0.0f, ZAxis.x, Position->x );
    Output->E[1] = V4( XAxis.y, 1.0f, ZAxis.y, Position->y );
    Output->E[2] = V4( XAxis.z, 0.0f, ZAxis.z, Position->z );
    Output->E[3] = V4(    0.0f, 0.0f,    0.0f, 1.0f );
}

inline void
PerspectiveFovLH(matrix *Output, 
                 r32 FovAngleY, r32 AspectRatio, 
                 r32 NearZ, r32 FarZ)
{
    // REFERENCE:
    // MSDN: win32/dxtecharts/the-direct3d-transformation-pipeline
    r32 X22 = 1.0f/tan(FovAngleY/2);
    r32 X11 = X22*AspectRatio;
    r32 X33 = FarZ/(FarZ - NearZ);
    r32 X34 = 1.0f;
    r32 X43 = -NearZ*FarZ/(FarZ - NearZ);;
    
    Output->E[0] = V4( X11,   0,   0,   0 );
    Output->E[1] = V4(   0, X22,   0,   0 );
    Output->E[2] = V4(   0,   0, X33, X34 );
    Output->E[3] = V4(   0,   0, X43,   0 );
}

inline void
PerspectiveOrthLH(matrix *Output, r32 Width, r32 Height, r32 NearZ, r32 FarZ)
{
    r32 X11 = 2.0f / Width;
    r32 X22 = 2.0f / Height;
    r32 X33 = 1.0f / (FarZ - NearZ);
    r32 X43 = -NearZ / (FarZ - NearZ);
    
    Output->E[0] = V4( X11,   0,   0, 0 );
    Output->E[1] = V4(   0, X22,   0, 0 );
    Output->E[2] = V4(   0,   0, X33, 0 );
    Output->E[3] = V4(   0,   0, X43, 1 );
}

inline v2
GetWorldToScreenEx(game_camera &camera, v3 *position, u32 screenWidth, u32 screenHeight)
{
    v3 vector = TransformCoordV3(&Transpose(&camera.wvp), position);
    
    r32 Sx = (vector.x + 1.0f) * screenWidth/2.0f;
    r32 Sy = (1.0f - vector.y) * screenHeight/2.0f;
    
    return {Sx, Sy};
}

/*
Ray GetMouseRay(Vector2 mousePosition, Camera camera);
matrix GetCameramatrix(Camera camera);
matrix GetCameramatrix2D(Camera2D camera);
Vector2 GetWorldToScreen(Vector3 position, Camera camera);
Vector2 GetWorldToScreen2D(Vector2 position, Camera2D camera);
Vector2 GetScreenToWorld2D(Vector2 position, Camera2D camera);
*/

#endif //CAMERA_H
