#ifndef WAH_H
#define WAH_H

struct task;

void wah_init(void (*entry)(void));
void wah_start_beacon(void);
void wah_connect_loop(void);
int wah_is_superuser(struct task *t);
void wah_audit(const char *action, struct task *t);
void wah_set_trust(struct task *t, int score);
int  wah_get_trust(struct task *t);
void wah_set_env_mode(int mode);
int  wah_get_env_mode(void);
/* Wah background learning routine invoked periodically */
void wah_self_evolve(void);
/* Return nonzero if message passes emotional filtering */
int  wah_emotion_filter(const char *msg);
/* Return nonzero if metacognitive checks approve action */
int  wah_metacog_check(const char *msg);
void wah_vm_start(const char *image);
void wah_vm_stop(void);
void wah_send_log(void);
void wah_send_time(void);
void wah_send_rtc(void);
void wah_send_random(void);
void wah_send_tasks(void);
void wah_send_devices(void);
void wah_send_names(void);
void wah_session_start(const char *image, int trust);
int  wah_session_is_active(void);
const char *wah_session_image(void);
int  wah_session_trust(void);

#endif
