/*****************************************************************************
        FILE : submit.c (Assignment 1)
        NOTE : you have to implement functions in this file
*****************************************************************************/
/*****************************************************************************
		Student Information (非常重要)
		Student ID (学号):162110212
		Student Name(姓名):滕林
*****************************************************************************/
//导包
#include <cstdlib>
#include<iostream>
#include <GL/glut.h>
#include<cmath>
#define BMP_Header_Length 54                   //图像数据在内存块中的偏移量
using namespace std;
const GLdouble FRUSTDIM = 100.0f;

/*定义键鼠操作的一些全局变量*/
int option=0;                                                   //要进行的操作
bool enableLight = 1;                                       //灯光开关
GLfloat window_rot=0.f;                                 //窗户旋转角度
GLfloat wardrobe_rot=0.f;                              //柜子的旋转角度
GLfloat drawer_dis=0.f;                                 //抽屉的抽出距离
GLfloat ban_dis=0.f;                                        //放键盘的滑动板偏移距离
GLfloat chair_surface_angle=0.f;                   //椅子表面的角度
GLfloat air_conditioning_rot=0.f;                   //空调扇叶旋转角度
GLfloat chair_dis=0.f;                                      //椅子偏移距离
int tv_cru_color=5;                                         //电视当前的节目
bool flag=0;                                                    //灯的开关

/*定义纹理属性编号*/
GLuint window_tex;
GLuint tv_tex[5];

/*定义光源参数*/
GLfloat light0pos[] = {0.0f, 76.f, 0.f, 0.f};
GLfloat light0_mat1[] = {0.0, 0.0, 0.0, 0.f};
GLfloat light0_diff[] = {1.0, 1.0, 1.0, 0.3};

/*用于判断一个整数是不是2的整数次幂*/
int power_of_two(int n)
{
    if (n <= 0)
        return 0;
    return (n & (n - 1)) == 0;
}
/*用于加载纹理图片*/
GLuint load_texture(const char* file_name)
{
    GLint width, height, total_bytes;
    GLubyte* pixels = 0;
    GLuint last_texture_ID = 0, texture_ID = 0;

    // 打开文件，如果失败，返回
    FILE* pFile = fopen(file_name, "rb");
    if (pFile == 0)
        return 0;

    // 读取文件中图象的宽度和高度
    fseek(pFile, 0x0012, SEEK_SET);
    fread(&width, 4, 1, pFile);
    fread(&height, 4, 1, pFile);
    printf("%d %d", width, height);
    fseek(pFile, BMP_Header_Length, SEEK_SET);

    // 计算每行像素所占字节数，并根据此数据计算总像素字节数
    {
        GLint line_bytes = width * 3;
        while (line_bytes % 4 != 0)
            ++line_bytes;
        total_bytes = line_bytes * height;
    }

    // 根据总像素字节数分配内存
    pixels = (GLubyte*)malloc(total_bytes);
    if (pixels == 0)
    {
        fclose(pFile);
        return 0;
    }

    // 读取像素数据
    if (fread(pixels, total_bytes, 1, pFile) <= 0)
    {
        free(pixels);
        fclose(pFile);
        return 0;
    }

    // 对就旧版本的兼容，如果图象的宽度和高度不是的整数次方，则需要进行缩放
    // 若图像宽高超过了OpenGL规定的最大值，也缩放
    {
        GLint max;
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max);
        if (!power_of_two(width)
            || !power_of_two(height)
            || width > max
            || height > max)
        {
            const GLint new_width = 256;
            const GLint new_height = 256; // 规定缩放后新的大小为边长的正方形
            GLint new_line_bytes, new_total_bytes;
            GLubyte* new_pixels = 0;

            // 计算每行需要的字节数和总字节数
            new_line_bytes = new_width * 3;
            while (new_line_bytes % 4 != 0)
                ++new_line_bytes;
            new_total_bytes = new_line_bytes * new_height;

            // 分配内存
            new_pixels = (GLubyte*)malloc(new_total_bytes);
            if (new_pixels == 0)
            {
                free(pixels);
                fclose(pFile);
                return 0;
            }

            // 进行像素缩放
            gluScaleImage(GL_RGB,
                          width, height, GL_UNSIGNED_BYTE, pixels,
                          new_width, new_height, GL_UNSIGNED_BYTE, new_pixels);

            // 释放原来的像素数据，把pixels指向新的像素数据，并重新设置width和height
            free(pixels);
            pixels = new_pixels;
            width = new_width;
            height = new_height;
        }
    }

    // 分配一个新的纹理编号
    glGenTextures(1, &texture_ID);
    if (texture_ID == 0)
    {
        free(pixels);
        fclose(pFile);
        return 0;
    }

    // 绑定新的纹理，载入纹理并设置纹理参数
    // 在绑定前，先获得原来绑定的纹理编号，以便在最后进行恢复
    GLint lastTextureID = last_texture_ID;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &lastTextureID);
    glBindTexture(GL_TEXTURE_2D, texture_ID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0,
                 GL_BGR_EXT, GL_UNSIGNED_BYTE, pixels);
    glBindTexture(GL_TEXTURE_2D, lastTextureID);  //恢复之前的纹理绑定
    free(pixels);
    return texture_ID;
}
/*开启设置内容*/
void init()
{
    glLightfv(GL_LIGHT0, GL_POSITION, light0pos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light0_mat1);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diff);

    glEnable(GL_DEPTH_TEST);                                                                                    //开启深度测试，进行渲染操作
    glEnable(GL_LIGHTING);                                                                                         //开启光照
    glEnable(GL_LIGHT0);                                                                                              //开启0号光源
    glEnable(GL_NORMALIZE);
    glEnable(GL_COLOR_MATERIAL);                                                                            //开启颜色材质
    glFrontFace(GL_CCW);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    window_tex = load_texture("./chuangwai.bmp");                                               //加载电视纹理
    char url[]="./tv_pic/x.bmp";
    for(int i=0;i<5;i++){
        url[9]=i+'0'-0;
        tv_tex[i]=load_texture(url);
    }
}
/*绘制指定的四边形*/
void ChairLeg()
{
    glRotatef(90, 0, -1, 0);
    glRotatef(20, -1, 0, 0);
    glBegin(GL_QUAD_STRIP);                                                                 //填充凸多边形4个点绘制一个四边形，6个点绘制2个四边形，8个点绘制3个四边形；
    glVertex3f(0, 0, 0);
    glVertex3f(0, 10, 0);
    glVertex3f(10, 0, 0);
    glVertex3f(10, 10, 0);
    glVertex3f(7, 3, -60);
    glVertex3f(7, 7, -60);
    glVertex3f(3, 3, -60);
    glVertex3f(3, 7, -60);
    glEnd();
    glBegin(GL_QUAD_STRIP);
    glVertex3f(0, 0, 0);
    glVertex3f(10, 0, 0);
    glVertex3f(3, 3, -60);
    glVertex3f(7, 3, -60);
    glVertex3f(0, 10, 0);
    glVertex3f(10, 10, 0);
    glVertex3f(3, 7, -60);
    glVertex3f(7, 7, -60);
    glEnd();
}
/*主要的绘图函数*/
void display()
{
    /*清空缓存*/
    glClearColor(0.0,0.0,0.0,1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    /*加载画板*/
    glLoadIdentity();
    glPushMatrix();
    glTranslatef(0, 0, -550);

    /*设置纹理与材质*/
    GLfloat no_mat[] = {0.0,0.0,0.0,1.0};
    GLfloat mat_diffuse[] = {0.9,0.9,0.9,1.0};
    GLfloat mat_specular[] = {0.3,0.3,0.3,1.0};
    GLfloat high_shininess[] = {20.0};
    glMaterialfv(GL_FRONT, GL_AMBIENT, no_mat);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, high_shininess);
    glMaterialfv(GL_FRONT, GL_EMISSION, no_mat);

    /* 画四周的墙 */
    glPushMatrix();
    //（1） 后方的墙
    glBegin(GL_QUADS);
    glColor3f(0.351f,0.46f,0.55f);
    glVertex3f(-160.0f, 120.0f, -200.0f);
    glVertex3f(160.0f, 120.0f, -200.0f);
    glVertex3f(160.0f, -120.0f, -200.0f);
    glVertex3f(-160.0f, -120.0f,-200.0f);
    //（2） 上方的墙
    glColor3f(0.651f,0.66f,0.665f);
    glVertex3f(-160.0f, 120.0f, -200.0f);
    glVertex3f(160.0f, 120.0f, -200.0f);
    glVertex3f(240.0f, 160.0f,0.0f);
    glVertex3f(-240.0f, 160.0f, 0.0f);
    //（3） 下方的墙
    glColor3f(0.15f,0.14f,0.27f);
    glVertex3f(-160.f, -120.f, -200.f);
    glVertex3f(160.f, -120.f, -200.f);
    glVertex3f(240.f, -160.f, 0.f);
    glVertex3f(-240.f, -160.f, 0.f);
    //（4）右边的墙
    glColor3f(0.468f,0.5f,0.57f);
    glVertex3f(160.0f, 120.0f, -200.0f);
    glVertex3f(190.0f, 160.0f, 0.0f);
    glVertex3f(190.0f, -160.0f, 0.0f);
    glVertex3f(160.0f, -120.0f, -200.0f);
    //（5）左边的墙
    glColor3f(0.468f,0.5f,0.57f);
    glVertex3f(-160.0f, 120.0f, -200.0f);
    glVertex3f(-160.0f, -120.0f, -200.0f);
    glVertex3f(-190.0f, -160.0f, 0.0f);
    glVertex3f(-190.0f, 160.0f, 0.0f);
    glEnd();

    /*画窗户*/
    //(0)设置填充图片（暂时用黑白替代）
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, window_tex);
    glBegin(GL_QUADS);
    glTexCoord2f(1,0);glVertex3f(-20.0,0.0,-197.0);
    glTexCoord2f(0,0);glVertex3f(-120.0,0.0,-197.0);
    glTexCoord2f(0,1);glVertex3f(-120.0,70.0,-197.0);
    glTexCoord2f(1,1);glVertex3f(-20.0,70.0,-197.0);
    glEnd();
    glDisable(GL_TEXTURE_2D);
    glPushMatrix();
    glTranslatef(-120.0,0.0,-195.0);                                         //更改旋转轴
    glRotatef(-window_rot, 0.0, 1.0, 0.0);                   // 绕y轴旋转30度
    glTranslatef(120.0,0.0,195.0);
    //（1）窗户外轮廓
    glBegin(GL_QUADS);
    glColor3f(0.3f,0.4f,0.5f);
    glVertex3f(-70.0,0.0,-195.0);
    glVertex3f(-120.0,0.0,-195.0);
    glVertex3f(-120.0,70.0,-195.0);
    glVertex3f(-70.0,70.0,-195.0);
    //（2）窗户内轮廓
    glColor3f(0.6f,0.7f,0.8f);
    glVertex3f(-75.0,5.0,-190.0);
    glVertex3f(-115.0,5.0,-190.0);
    glVertex3f(-115.0,65.0,-190.0);
    glVertex3f(-75.0,65.0,-190.0);
    //（3）画出窗户把手
    glColor3f(0.1,0.1,0.1);
    glVertex3f(-71.0,31.0,-185.0);
    glVertex3f(-71.0,39.0,-185.0);
    glVertex3f(-74.0,39.0,-185.0);
    glVertex3f(-74.0,31.0,-185.0);
    glEnd();
    glPopMatrix();
    //（4）在右边复制同一份
    glPushMatrix();
    glTranslatef(-20.0,0.0,-195.0);                                         //更改旋转轴
    glRotatef(window_rot, 0.0, 1.0, 0.0);                   // 绕y轴旋转30度
    glTranslatef(20.0,0.0,195.0);
    glBegin(GL_QUADS);
    glColor3f(0.3f,0.4f,0.5f);
    glVertex3f(-70.0,0.0,-195.0);
    glVertex3f(-70.0,70.0,-195.0);
    glVertex3f(-20.0,70.0,-195.0);
    glVertex3f(-20.0,0.0,-195.0);
    glColor3f(0.6f,0.7f,0.8f);
    glVertex3f(-65.0,5.0,-190.0);
    glVertex3f(-65.0,65.0,-190.0);
    glVertex3f(-25.0,65.0,-190.0);
    glVertex3f(-25.0,5.0,-190.0);
    glColor3f(0.1,0.1,0.1);
    glVertex3f(-69.0,31.0,-185.0);
    glVertex3f(-69.0,39.0,-185.0);
    glVertex3f(-66.0,39.0,-185.0);
    glVertex3f(-66.0,31.0,-185.0);
    glEnd();
    glPopMatrix();

    /*床*/
   glPushMatrix();
   //（1）床体
   glBegin(GL_QUADS);
   //里面
   glColor3f(0.65, 0.72, 0.75);
    glVertex3f(-20.0, -120.0, -190.0);
    glVertex3f(-20.0,  -90.0, -190.0);
    glVertex3f(159.0, -90.0, -190.0);
    glVertex3f(159.0, -120.0, -190.0);
   //外面
    glVertex3f(-20.0, -120.0, -50.0);
    glVertex3f(-20.0,  -90.0, -50.0);
    glVertex3f(159.0, -90.0, -50.0);
    glVertex3f(159.0, -120.0, -50.0);
    //底面
    glVertex3f(-20.0,-120.0,-50.0);
    glVertex3f(-20.0,-120.0,-190.0);
    glVertex3f(159.0,-120.0,-190.0);
    glVertex3f(159.0,-120.0,-50.0);
    //左侧面
    glVertex3f(-20.0,-120.0,-50.0);
    glVertex3f(-20.0,-90.0,-50.0);
    glVertex3f(-20.0,-90.0,-190.0);
    glVertex3f(-20.0,-120.0,-190.0);
    //右边侧面
    glVertex3f(159.0,-120.0,-50.0);
    glVertex3f(159.0,-90.0,-50.0);
    glVertex3f(159.0,-90.0,-190.0);
    glVertex3f(159.0,-120.0,-190.0);
    //上顶面
    glVertex3f(-20.0,-90.0,-50.0);
    glVertex3f(-20.0,-90.0,-190.0);
    glVertex3f(159.0,-90.0,-190.0);
    glVertex3f(159.0,-90.0,-50.0);
   glEnd();
   //（2）床的支撑柱子
   //外左边柱子
    glPushMatrix();
    glColor3f(0.5f, 0.4f, 0.2f);
    glTranslatef(-15.f, -130.f, -50.f);
    glRotatef(-90, 1.f, 0.f, 0.f);
    GLUquadricObj *quadobj=gluNewQuadric();
    gluCylinder(quadobj, 3.f, 3.f, 10.f, 20.f, 20.f);
    glPopMatrix();
    //外右边柱子
    glPushMatrix();
    glColor3f(0.168f, 0.23f, 1.0f);
    glTranslatef(155.f, -130.f, -50.f);
    glRotatef(-90, 1.f, 0.f, 0.f);
    quadobj=gluNewQuadric();
    gluCylinder(quadobj, 3.f, 3.f, 10.f, 20.f, 20.f);
    glPopMatrix();
    //（3）床头板
    glPushMatrix();
    glBegin(GL_QUADS);
    glColor3f(0.5f, 0.4f, 0.3f);
    //里面的近似三角形的梯形
    glVertex3f(155.0,-50.0,-190.0);
    glVertex3f(159.0,-50.0,-190.0);
    glVertex3f(159.0,-90.0,-190.0);
    glVertex3f(145.0,-90.0,-190.0);
    //外面的
    glVertex3f(155.0,-50.0,-50.0);
    glVertex3f(159.0,-50.0,-50.0);
    glVertex3f(159.0,-90.0,-50.0);
    glVertex3f(145.0,-90.0,-50.0);
    //上面的长方形的顶
    glVertex3f(155.0,-50.0,-190.0);
    glVertex3f(159.0,-50.0,-190.0);
    glVertex3f(159.0,-50.0,-50.0);
    glVertex3f(155.0,-50.0,-50.0);
    //左侧面的长方形
    glVertex3f(145.0,-90.0,-190.0);
    glVertex3f(155.0,-50.0,-190.0);
    glVertex3f(155.0,-50.0,-50.0);
    glVertex3f(145.0,-90.0,-50.0);

    glEnd();
    glPopMatrix();


    /*画一个电视*/
    glBegin(GL_QUADS);
    //（1）画里面
    glColor3f(0.1,0.1,0.1);
    glVertex3f(-159.0,10.0,-150.0);
    glVertex3f(-159.0,-50.0,-150.0);
    glVertex3f(-159.0,-50.0,0.0);
    glVertex3f(-159.0,10.0,0.0);
    //（3）画上面
    glColor3f(0.3,0.3,0.3);
    glVertex3f(-159.0,10.0,-150.0);
    glVertex3f(-157.0,10.0,-150.0);
    glVertex3f(-157.0,10.0,0.0);
    glVertex3f(-159.0,10.0,0.0);
    //（4）画下面
    glVertex3f(-159.0,-50.0,-150.0);
    glVertex3f(-157.0,-50.0,-150.0);
    glVertex3f(-157.0,-50.0,0.0);
    glVertex3f(-159.0,-50.0,0.0);
    //（5）画左面
    glVertex3f(-159.0,-50.0,0.0);
    glVertex3f(-159.0,10.0,0.0);
    glVertex3f(-157.0,10.0,0.0);
    glVertex3f(-157.0,-50.0,0.0);
    //（6）画右面
    glVertex3f(-159.0,-50.0,-150.0);
    glVertex3f(-159.0,10.0,-150.0);
    glVertex3f(-157.0,10.0,-150.0);
    glVertex3f(-157.0,-50.0,-150.0);
    glEnd();
    //（2）画外面
    if(tv_cru_color==5){
        glBegin(GL_QUADS);
        glColor3f(0.0,0.0,0.0);
        glVertex3f(-157.0,10.0,-150.0);
        glVertex3f(-157.0,-50.0,-150.0);
        glVertex3f(-157.0,-50.0,0.0);
        glVertex3f(-157.0,10.0,0.0);
        glEnd();
    }else{
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, tv_tex[tv_cru_color]);
        glBegin(GL_QUADS);
        glTexCoord2f(1,0);glVertex3f(-157.0,10.0,-150.0);
        glTexCoord2f(0,0);glVertex3f(-157.0,-50.0,-150.0);
        glTexCoord2f(0,1); glVertex3f(-157.0,-50.0,0.0);
        glTexCoord2f(1,1);glVertex3f(-157.0,10.0,0.0);
        glEnd();
        glDisable(GL_TEXTURE_2D);
    }
    glPopMatrix();

   /*开始画衣柜*/
    glPushMatrix();
    glColor3f(0.57, 0.56, 0.58);
    glBegin(GL_QUADS);
    //（1）画背板
    glVertex3f(159.0,-130.0,-45.0);
    glVertex3f(159.0,30.0,-45.0);
    glVertex3f(159.0,30.0,50.0);
    glVertex3f(159.0,-130.0,50.0);
    //（2）画外板
    glVertex3f(130.0,-130.0,50.0);
    glVertex3f(130.0,30.0,50.0);
    glVertex3f(159.0,30.0,50.0);
    glVertex3f(159.0,-130.0,50.0);
    //（3）画里板
    glVertex3f(130.0,-130.0,-45.0);
    glVertex3f(130.0,30.0,-45.0);
    glVertex3f(159.0,30.0,-45.0);
    glVertex3f(159.0,-130.0,-45.0);
    //（4）画下板
    glVertex3f(130.0,30.0,-45.0);
    glVertex3f(159.0,30.0,-45.0);
    glVertex3f(159.0,30.0,50.0);
    glVertex3f(130.0,30.0,50.0);
    //（5）画上板
    glVertex3f(130.0,-130.0,-45.0);
    glVertex3f(130.0,-130.0,-45.0);
    glVertex3f(130.0,-130.0,50.0);
    glVertex3f(130.0,-130.0,50.0);
    glEnd();
    glPopMatrix();
    //（6）画前板的左右开关柜
    glPushMatrix();
    glColor3f(0.67, 0.66, 0.68);
    glTranslatef(130.0,-60.0,-45.0);                                          //更改旋转轴
    glRotatef(-wardrobe_rot, 0.0, 1.0, 0.0);                   // 绕y轴旋转30度
    glTranslatef(-130.0,60.0,45.0);
    glBegin(GL_QUADS);
    glVertex3f(130.0,-60.0,-45.0);
    glVertex3f(130.0,30.0,-45.0);
    glVertex3f(130.0,30.0,2.0);
    glVertex3f(130.0,-60.0,2.0);

    glColor3f(0.1,0.1,0.1);
    glVertex3f(129,-25,-10.0);
    glVertex3f(129,-5,-10.0);
    glVertex3f(129,-5,0.0);
    glVertex3f(129,-25,0.0);
    glEnd();
    glPopMatrix();
    glPushMatrix();
    glColor3f(0.67, 0.66, 0.68);
    glTranslatef(130.0,-60.0,50.0);                                          //更改旋转轴
    glRotatef(wardrobe_rot, 0.0, 1.0, 0.0);                   // 绕y轴旋转30度
    glTranslatef(-130.0,60.0,-50.0);
    glBegin(GL_QUADS);
    glVertex3f(130.0,-60.0,3.0);
    glVertex3f(130.0,30.0,3.0);
    glVertex3f(130.0,30.0,50.0);
    glVertex3f(130.0,-60.0,50.0);
    glColor3f(0.1,0.1,0.1);
    glVertex3f(129,-25,14.0);
    glVertex3f(129,-5,14.0);
    glVertex3f(129,-5,4.0);
    glVertex3f(129,-25,4.0);
    glEnd();
    glPopMatrix();
    //（7）画抽屉
    for(int i=0;i<3;i++){
    glPushMatrix();
    glColor3f(0.77, 0.76, 0.78);
    glBegin(GL_QUADS);
    //画里板
    glVertex3f(130.0-drawer_dis,-80.0-i*20.0,-44.0);
    glVertex3f(130.0-drawer_dis,-61.0-i*20.0,-44.0);
    glVertex3f(159.0-drawer_dis,-61.0-i*20.0,-44.0);
    glVertex3f(159.0-drawer_dis,-80.0-i*20.0,-44.0);
    //画外板
    glVertex3f(130.0-drawer_dis,-80.0-i*20.0,49.0);
    glVertex3f(130.0-drawer_dis,-61.0-i*20.0,49.0);
    glVertex3f(159.0-drawer_dis,-61.0-i*20.0,49.0);
    glVertex3f(159.0-drawer_dis,-80.0-i*20.0,49.0);
    //画底板
    glColor3f(0.5,0.5,0.5);
    glVertex3f(130.0-drawer_dis,-80.0-i*20.0,-44.0);
    glVertex3f(159.0-drawer_dis,-80.0-i*20.0,-44.0);
    glVertex3f(159.0-drawer_dis,-80.0-i*20.0,49.0);
    glVertex3f(130.0-drawer_dis,-80.0-i*20.0,49.0);
    //画外板
    glColor3f(1.0,1.0,1.0);
    glVertex3f(130.0-drawer_dis,-80.0-i*20.0,-44.0);
    glVertex3f(130.0-drawer_dis,-61.0-i*20.0,-44.0);
    glVertex3f(130.0-drawer_dis,-61.0-i*20.0,49.0);
    glVertex3f(130.0-drawer_dis,-80.0-i*20.0,49.0);
    //画里板
    glVertex3f(159.0-drawer_dis,-80.0-i*20.0,-44.0);
    glVertex3f(159.0-drawer_dis,-61.0-i*20.0,-44.0);
    glVertex3f(159.0-drawer_dis,-61.0-i*20.0,49.0);
    glVertex3f(159.0-drawer_dis,-80.0-i*20.0,49.0);
    //画把手
    glColor3f(0.1,0.1,0.1);
    glVertex3f(128.0-drawer_dis,-71.0-i*20.0,-5.0);
    glVertex3f(128.0-drawer_dis,-69.0-i*20.0,-5.0);
    glVertex3f(128.0-drawer_dis,-69.0-i*20.0,10.0);
    glVertex3f(128.0-drawer_dis,-71.0-i*20.0,10.0);
    glEnd();
    glPopMatrix();
    }

    /*画一个灯*/
    glPushMatrix();
    //（1）画灯的圆柱形四周面
    glColor3f(0.8,0.8,0.8);
    glTranslatef(0.0, 130.0, -100.f);
    glRotatef(-90, 1.f, 0.f, 0.f);
    quadobj=gluNewQuadric();
    gluCylinder(quadobj, 30.f, 30.f, 10.f, 20.f, 20.f);
    glPopMatrix();
    //（2）画灯的下面罩
    glColor3f(1.0,1.0,1.0);
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0.0, 130.0,-100.0); // 圆心坐标
    glVertex3f(30.0, 130.0,-100.0); // 圆上一点坐标
    for (float angle = 0.0f; angle <= 360.0f; angle += 0.1f) {
        glVertex3f(cos(angle) * 30.0f, 130.0,sin(angle) * 30.0f-100.0); // 圆上任一点坐标
    }
    glEnd();

    /*画一个书桌*/
    glPushMatrix();
    glBegin(GL_QUADS);
    //画一个左腿
    glColor3f(0.9,0.7,0.5);
    glVertex3f(-170.0,-140.0,30.0);
    glVertex3f(-170.0,-80.0,30.0);
    glVertex3f(-130.0,-80.0,30.0);
    glVertex3f(-130.0,-140.0,30.0);
    //画一个右腿
    glVertex3f(-170.0,-140.0,-50.0);
    glVertex3f(-170.0,-80.0,-50.0);
    glVertex3f(-130.0,-80.0,-50.0);
    glVertex3f(-130.0,-140.0,-50.0);
    //画一个上顶
    glColor3f(0.9,0.8,0.6);
    glVertex3f(-170.0,-80.0,35.0);
    glVertex3f(-170.0,-80.0,-55.0);
    glVertex3f(-130.0,-80.0,-55.0);
    glVertex3f(-130.0,-80.0,35.0);
    glColor3f(1.0,1.0,1.0);
    //画一个放电脑键盘的滑动板
    glColor3f(0.8,0.8,0.7);
    glVertex3f(-170.0+ban_dis,-100.0,30.0);
    glVertex3f(-170.0+ban_dis,-100.0,-50.0);
    glVertex3f(-130.0+ban_dis,-100.0,-50.0);
    glVertex3f(-130.0+ban_dis,-100.0,30.0);
    glEnd();
    glPopMatrix();

    /*椅子*/
    glPushMatrix();
    glTranslatef(-80+chair_dis, -115, 50 );
    glRotatef(0.0, 0, 1, 0);
    glScalef(0.4, 0.4, 0.4);
    //（1）表面
    glPushMatrix();
    glRotatef(chair_surface_angle, 0, 1, 0);
    glPushMatrix();
    glColor3f(0.1, 0.1, 0.1);
    glPushMatrix();
    glTranslatef(0, 50, 0);
    glScalef(55, 10, 85);
    glutSolidCube(1);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(40, 47, 0);
    glRotatef(12, 0, 0, -1);
    glScalef(30, 10, 85);
    glutSolidCube(1);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(10, 0, 0);
    glPushMatrix();
    glTranslatef(-43, 55, 0);
    glRotatef(24, 0, 0, -1);
    glScalef(20, 10, 85);
    glutSolidCube(1);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(-53, 65, 0);
    glRotatef(60, 0, 0, -1);
    glScalef(20, 10, 85);
    glutSolidCube(1);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(-56, 75, 0);
    glRotatef(85, 0, 0, -1);
    glScalef(20, 10, 85);
    glutSolidCube(1);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(-56, 120, 0);
    glRotatef(90, 0, 0, -1);
    glScalef(80, 10, 85);
    glutSolidCube(1);
    glPopMatrix();
    glPopMatrix();
    glPopMatrix();
    glPopMatrix();
    //（2）支撑
    glPushMatrix();
    glColor3f(0.1, 0.1, 0.1);
    glTranslatef(-80+chair_dis, -120, 50);
    glScalef(0.5, 0.5, 0.5);
    glRotatef(-90, 1, 0, 0);
    gluCylinder(quadobj, 6, 6, 50, 20, 20);
    glPopMatrix();
    //（3）轮子
    glPushMatrix();
    glColor3f(0.1, 0.1, 0.1);
    glTranslatef(-80+chair_dis, -120, 50);
    glScalef(0.5, 0.5, 0.5);
    glPushMatrix();
    ChairLeg();
    glPopMatrix();
    glPushMatrix();
    glTranslatef(55, -20, 5);
    glutSolidSphere(5, 50, 50);
    glPopMatrix();
    glPushMatrix();//1
    glRotatef(72, 0, -1, 0);
    ChairLeg();
    glPopMatrix();
    glPushMatrix();
    glTranslatef(14, -20, 52);
    glutSolidSphere(5, 50, 50);
    glPopMatrix();
    glPushMatrix();//1
    glRotatef(144, 0, -1, 0);
    ChairLeg();
    glPopMatrix();
    glPushMatrix();
    glTranslatef(-48, -20, 29);
    glutSolidSphere(5, 50, 50);
    glPopMatrix();
    glPushMatrix();//1
    glRotatef(216, 0, -1, 0);
    ChairLeg();
    glPopMatrix();
    glPushMatrix();
    glTranslatef(-45, -20, -38);
    glutSolidSphere(5, 50, 50);
    glPopMatrix();
    glPushMatrix();//
    glRotatef(288, 0, -1, 0);
    ChairLeg();
    glPopMatrix();
    glPushMatrix();
    glTranslatef(20, -20, -52);
    glutSolidSphere(5, 50, 50);
    glPopMatrix();
    glPopMatrix();
    glPopMatrix();

    /*画一个空调*/
    glPushMatrix();
    glBegin(GL_QUADS);
    //（1）画左板
    glColor3f(0.9,0.9,0.9);
    glVertex3f(40.0,100.0,-180.0);
    glVertex3f(40.0,100.0,-130.0);
    glVertex3f(40.0,80.0,-130.0);
    glVertex3f(40.0,65.0,-180.0);
    //（2）画右板
    glVertex3f(120.0,100.0,-180.0);
    glVertex3f(120.0,100.0,-130.0);
    glVertex3f(120.0,80.0,-130.0);
    glVertex3f(120.0,65.0,-180.0);
    //（3）画上板
    glColor3f(0.6,0.6,0.6);
    glVertex3f(40.0,100.0,-180.0);
    glVertex3f(120.0,100.0,-180.0);
    glVertex3f(120.0,100.0,-130.0);
    glVertex3f(40.0,100.0,-130.0);
    //（4）画前板
    glColor3f(0.7,0.7,0.7);
    glVertex3f(40.0,100.0,-130.0);
    glVertex3f(120.0,100.0,-130.0);
    glVertex3f(120.0,80.0,-130.0);
    glVertex3f(40.0,80.0,-130.0);
    //（5）画侧板
    glColor3f(0.7,0.7,0.7);
    glVertex3f(40.0,65.0,-180.0);
    glVertex3f(40.0,75.0,-147.0);
    glVertex3f(120.0,75.0,-147.0);
    glVertex3f(120.0,65.0,-180.0);
    glEnd();
    glPopMatrix();
    //（6）画排风口
    glPushMatrix();
    glColor3f(0.6,0.6,0.622);
    glTranslatef(40.0,77.5,-138.4);                                            //更改旋转轴
    glRotatef(air_conditioning_rot, 1.0, 0.0, 0.0);
    glTranslatef(-40.0,-77.5,138.4);
    glBegin(GL_QUADS);
    glVertex3f(40.0,75.0,-147.0);
    glVertex3f(40.0,80.0,-130.0);
    glVertex3f(120.0,80.0,-130.0);
    glVertex3f(120.0,75.0,-147.0);
    glEnd();
    glPopMatrix();

   //完善纹理操作（窗户和电视）
    glPushMatrix();
    glBegin(GL_QUADS);

    glEnd();
    glPopMatrix();
    glPopMatrix();
    glutSwapBuffers();
    glFlush();
    glutPostRedisplay();
}
void reshape(int w, int h){
    glViewport(0, 0, (GLsizei) w, (GLsizei) h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-FRUSTDIM, FRUSTDIM, -FRUSTDIM, FRUSTDIM, 300., 800.);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}
/*键盘操作*/
void keyboard(unsigned char key, int x, int y){
    switch (key)
    {
        case'\033':                                                                                                  //退出程序
            exit(0);
        case ' ':                                                                                                       //灯的开关
            for (int i=0; i<=3 ; i++){
                if(flag==1){
                    light0_mat1[i] =0.0;
                    light0_diff[i] = 0.0;
                }else{
                    light0_mat1[i] =1.0;
                    light0_diff[i] =1.0;
                }
            }
            flag=1-flag;
            glLightfv(GL_LIGHT0, GL_AMBIENT, light0_mat1);
            glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diff);
            glEnable(GL_LIGHT0);
            break;
        case '1':                                                                                                        //增强灯光
            if (enableLight)
                if(light0_mat1[0]<1.0){
                    for (int i=0; i<=3 ; i++){
                        light0_mat1[i] += 0.01;
                        light0_diff[i] += 0.01;
                    }
                    glLightfv(GL_LIGHT0, GL_AMBIENT, light0_mat1);
                    glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diff);
                    glEnable(GL_LIGHT0);
                }
            break;
        case '2':                                                                                                       //减弱灯光
            if (enableLight)
                if(light0_mat1[0] > 0){
                    for (int i=0; i<=3 ; i++){
                        light0_mat1[i] -= 0.01;
                        light0_diff[i] -= 0.01;
                    }
                    glLightfv(GL_LIGHT0, GL_AMBIENT, light0_mat1);
                    glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diff);
                    glEnable(GL_LIGHT0);
                }
            break;
        case '3':                                                                                                       //打开窗户(每次打开15度，最多九十度)
            if(window_rot<90){
                window_rot+=1;
            }
            break;
        case '4':                                                                                                       //关上窗户（直接关上）
            if(window_rot>0){
                window_rot-=1;
            }
            break;
        case '5':                                                                                                        //第一次按，打开电视，第二次以后是换台
            if(tv_cru_color>=4){
                tv_cru_color=0;
            }else{
                tv_cru_color++;
            }
            break;
        case '6':                                                                                                       //关闭电视
            tv_cru_color=5;
            break;
        case '7':                                                                                                       //打开柜子上面
            if(wardrobe_rot<90){
                wardrobe_rot+=1;
            }
            break;
        case '8':                                                                                                        //关闭柜子上面
            if(wardrobe_rot>0){
                wardrobe_rot-=1;
            }
            break;
        case '9':                                                                                                       //打开柜子下面
            if(drawer_dis<20.0){
                drawer_dis+=0.5;
            }
            break;
        case '0':                                                                                                       //关闭柜子下面
            if(drawer_dis>0.0){
                drawer_dis-=0.5;
            }
            break;
        case 'a':                                                                                                       //打开键盘板子
            if(ban_dis<15.0){
                ban_dis+=0.5;
            }
            break;
        case 'b':                                                                                                       //关闭键盘板子
            if(ban_dis>0.0){
                ban_dis-=0.5;
            }
            break;
        case 'c':                                                                                                       //椅子逆时针旋转
            chair_surface_angle+=1.0;
            break;
        case 'd':                                                                                                       //椅子顺时针旋转
            chair_surface_angle-=1.0;
            break;
        case 'e':                                                                                                        //空调扇叶开启
            if(air_conditioning_rot<40.0){
                air_conditioning_rot+=1;
            }
            break;
        case 'f':                                                                                                          //空调扇叶关闭
            if(air_conditioning_rot>0.0){
                air_conditioning_rot-=1;
            }
            break;
        case 'g':                                                                                                       //椅子移动
            if(chair_dis<40){
                chair_dis+=1;
            }
            break;
        case 'h':
            if(chair_dis>0){
                chair_dis-=1;
            }
    }
}
void idle()
{
}

/*主函数*/
int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE|GLUT_DEPTH);

    glutInitWindowPosition(100, 100);                                   //设置窗口的位置
    glutInitWindowSize(800, 800);                       //设置窗口的大小
    glutCreateWindow("my room");
    init();
    glutReshapeFunc(reshape);
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutIdleFunc(idle);

    glutMainLoop();
    return 0;
}