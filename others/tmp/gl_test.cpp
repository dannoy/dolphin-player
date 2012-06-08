void native_gl_resize(int w, int h)
{
       LOGI("native_gl_resize %d %d", w, h);
       //wait_if_no_frame();
       glDeleteTextures(1, &s_texture);
       GLuint *start = s_disable_caps;
       while (*start)
               glDisable(*start++);
       glEnable(GL_TEXTURE_2D);
       glGenTextures(1, &s_texture);
       glBindTexture(GL_TEXTURE_2D, s_texture);
       glTexParameterf(GL_TEXTURE_2D,
                       GL_TEXTURE_MIN_FILTER, GL_LINEAR);
       glTexParameterf(GL_TEXTURE_2D,
                       GL_TEXTURE_MAG_FILTER, GL_LINEAR);
       glShadeModel(GL_FLAT);
       check_gl_error("glShadeModel");
       glColor4x(0x10000, 0x10000, 0x10000, 0x10000);
       check_gl_error("glColor4x");
       //int rect[4] = {0, TEXTURE_HEIGHT, TEXTURE_WIDTH, -TEXTURE_HEIGHT};
       int rect[4] = {0, my_h, my_w ,-my_h};

       glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_CROP_RECT_OES, rect);
       check_gl_error("glTexParameteriv");
       s_w = w;
       s_h = h;
LOGI("native_gl_resize Ends here");
       //s_pixels = (uint16_t *)malloc(w * h * 2);
}

void native_gl_render()
{
       if (player_exit_var) return;
       //__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "native_gl_render");
       //Wait for pixels to be available in the Buffer

       //__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "C");
       wait_if_no_frame();
       //__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "D");

       glClear(GL_COLOR_BUFFER_BIT);
       glTexImage2D(GL_TEXTURE_2D,             /* target */
                       0,                      /* level */
                       GL_RGB,                 /* internal format */
                       my_w,                   /* width */
                       my_h,                   /* height */
                       0,                      /* border */
                       GL_RGB,                 /* format */
                       GL_UNSIGNED_SHORT_5_6_5,/* type */
                       pictq[my_rindex].buffer);               /* pixels */
       //__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "after gltexImage2d");
       check_gl_error("glTexImage2D");
       glDrawTexiOES(0, 0, 0, s_w, s_h);
       //glDrawTexiOES(0, 0, 0, my_w, my_h);
       check_gl_error("glDrawTexiOES");

       my_rindex++;
       if (my_rindex == PICTQ_SIZE) {
           my_rindex = 0;
       }

       //Reduce the frame count
       decr_size();

       calculate_frames_per_second();
}
