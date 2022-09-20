/* 构造与析构 */
mph_error_t mph_jtagintf_init(mph_jtag_handle *h)
mph_error_t mph_jtagintf_free(mph_jtag_handle *h)

mph_error_t mph_jtagintf_connect(mph_jtag_handle *h, char *uri) /* uri => mphjtag://host:port:[optional password] */
mph_error_t mph_jtagintf_close(mph_jtag_handle *h)
mph_error_t mph_jtagintf_getstatus(mph_jtag_handle *h, mph_jtag_status *p)
/*
    mph_jtag_para：
    // R/O parameters
    char *hw_ver;
    char *sw_ver;
    mph_apiversion api_ver;

    mph_connstate state; // IDLE （未连接设备），CONNECTED（已连接设备），ONLINE（已连接DUT），WIP（正在传输数据）
    mph_error_t nonsyncerr;
    bool hw_prefer_park; // 硬件是否倾向于填充JTAG frame

    mph_transport_features transport_features; // bitmap for negative edge/cJTAG support....
    mph_jtag_speed maxspeed;
    mph_jtag_speed minspeed;

    mph_jtag_devices *devices; // 扫描出的设备，包括idcode/ir length，max.32。

    uint16_t dut_vref; // 以毫伏表示的目标板参考电压

    // R/W parameters

    mph_transport transport; // JTAG/cJTAG/TWDI/Nuclei-Wire等传输方式选择，其中TWDI/Nuclei-Wire不支持JTAG操作
    char *transport_flags; // 直接传递给传输后端的非格式化字符串flags，TBD

    mph_jtag_speed speed;

    bool sw_enable_park; // 软件是否允许JTAG frame park
    bool sie_method; // 上升发上升收、上升发下降收、下降发下降收、下降发上升收（*）

*/

/*
    mph_jtag_devices
    
    uint8_t index //在JTAG链中的位置
    bool occupied //是否被其他客户端使用
    uint32_t idcode // IDCODE
    uint32_t irlen // IR长度
*/

mph_error_t mph_jtagintf_ioctl(mph_jtag_handle *h, mph_jtag_para p, ...) // VA_ARGS

mph_error_t mph_jtagintf_getadapters(mph_jtag_handle *h, mph_adapter **a) 
mph_error_t mph_jtagintf_openadapter(mph_jtag_handle *h, mph_adapter *a, bool lock_as_dedicated) /* 连接设备，可选是否独占设备 */

mph_error_t mph_jtagintf_opendut(mph_jtag_handle *h) /* 扫JTAG链 */

/* 同步API */
mph_error_t mph_jtagintf_scanir(mph_jtag_handle *h, mph_jtag_device_mask devmask, mph_jtag_state exitst, uint8_t **wir_value, uint8_t **rir_value, uint32_t *wir_length = 0); /* 同步写IR */
mph_error_t mph_jtagintf_scandr(mph_jtag_handle *h, mph_jtag_device_mask devmask, mph_jtag_state exitst, uint8_t **wdr_value, uint8_t **rdr_value, uint32_t *wdr_length); /* 同步写DR */
mph_error_t mph_jtagintf_runtest(mph_jtag_handle *h, mph_jtag_state st, uint32_t cycles); /* 在某个稳定状态等待n个周期 */
mph_error_t mph_jtagintf_runstate(mph_jtag_handle *h, mph_jtag_state *st, uint32_t len); /* 按某个路径移动TAP状态机 */

/* 异步API */
mph_error_t mph_jtagintf_submit_scanir(mph_jtag_handle *h, mph_jtag_submission *s, mph_jtag_device_mask devmask, mph_jtag_state exitst, uint8_t **wir_value, uint8_t **rir_value, uint32_t *wir_length = 0); /* 同步写IR */
mph_error_t mph_jtagintf_submit_scandr(mph_jtag_handle *h, mph_jtag_submission *s, mph_jtag_device_mask devmask, mph_jtag_state exitst, uint8_t **wdr_value, uint8_t **rdr_value, uint32_t *wdr_length); /* 同步写DR */
mph_error_t mph_jtagintf_submit_runst(mph_jtag_handle *h,  mph_jtag_submission *s, mph_jtag_state *st, uint32_t cycles); /* 在某个稳定状态等待n个周期 */
mph_error_t mph_jtagintf_submit_runstate(mph_jtag_handle *h, mph_jtag_submission *s, mph_jtag_state *st, uint32_t len); /* 按某个路径移动TAP状态机 */

/* 异步API流程控制 */
mph_error_t mph_jtagintf_init_submission(mph_jtag_handle *h, mph_jtag_submission *s);
mph_error_t mph_jtagintf_free_submission(mph_jtag_handle *h, mph_jtag_submission *s);
mph_error_t mph_jtagintf_run_submission(mph_jtag_handle *h, mph_jtag_submission *s);
mph_error_t mph_jtagintf_abort_submission(mph_jtag_handle *h, mph_jtag_submission *s);
mph_error_t mph_jtagintf_get_progress(mph_jtag_handle *h, mph_jtag_submission *s, float *percent);

/* Flash操作流程控制 */
mph_error_t mph_flashintf_bind(mph_flash_handle *h, mph_jtag_handle *j, mph_jtag_device_mask devmask); // 将某个设备绑定到Flash Interface上
mph_error_t mph_flashintf_free(mph_flash_handle *h);
mph_error_t mph_flashintf_probe(mph_flash_handle *h, char *flashflags); // 扫描Flash
mph_error_t mph_flashintf_getdevices(mph_flash_handle *h, mph_flash_device **fp); //获得Flash列表
mph_error_t mph_flashintf_ioctl(mph_flash_handle *h, mph_flash_device *f, int op, ...); // VA_ARGS
/* 擦除控制 */
mph_error_t mph_flashintf_eraseall(mph_flash_handle *h, mph_flash_device *f);
mph_error_t mph_flashintf_erase(mph_flash_handle *h, mph_flash_device *f, mph_flash_addr startaddr, mph_flash_addr size, mph_flash_addr *actualstart, mph_flash_addr *actualend);
/* 经典读写控制 */
mph_error_t mph_flashintf_read(mph_flash_handle *h, mph_flash_device *f, mph_flash_addr startaddr, mph_flash_addr size, void *buf);
mph_error_t mph_flashintf_write(mph_flash_handle *h, mph_flash_device *f, mph_flash_addr startaddr, mph_flash_addr size, void *buf);
mph_error_t mph_flashintf_verify(mph_flash_handle *h, mph_flash_device *f, mph_flash_addr startaddr, mph_flash_addr size, void *buf);
