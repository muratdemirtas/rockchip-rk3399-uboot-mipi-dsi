 Hello, friends...
     The current MIPI DSI driver provided by Rockchip Inc. in older U-Boot versions (v2023 and earlier) is not compatible with U-Boot’s uclass subsystems.(a waste of time if you want to develop). 
     
I have done extensive research and made modifications to several files to make it work with the latest U-Boot versions. I have tested it with U-Boot 2023.10, 2024.10 and v2025.rc3 on orange pi 4 lts, and it works perfectly. I'm attaching the necessary files for those who need them.
     
This u-boot driver is nearly identical to Rockchip's Linux kernel driver, uses same driver file but there some limitations because of u-boot system, but it correctly displays the splash image when vidconsole0 probed by u-boot..

Additionally, i want to say that, in linux kernel side, the same dw-rockchip-dsi driver in latest armbian distribitions works with from kernels 5.18 to latest kernel v6.6 and v.6.12. I tested it.

Tested with custom 400x1200 60hz 4 lane standart mipi dsi panel from chinese panel manufacturer. I attached all modified files as attachment. Good lucks.

![CEEA2DF1-FAC3-4358-9540-641B00F88071](https://github.com/user-attachments/assets/3a8308aa-f556-4663-9777-62f4a9f0b113)


![34CA6D19-B542-4522-AC41-83A07AD0E464](https://github.com/user-attachments/assets/ed0243c0-f983-4a25-8849-f84f97e2d8aa)


 dm tree command output from u-boot is 

 reset         1  [ + ]   rockchip_reset        |   `-- reset                                                               
 syscon       27  [ + ]   rk3399_syscon         |-- syscon@ff770000                                                         
 nop           3  [ + ]   rockchip_iodomain     |   `-- io-domains                                                          
 video         0  [ + ]   rk3399_vop            |-- vop@ff8f0000                                                            
 vidconsole    0  [ + ]   vidconsole0           |   `-- vop@ff8f0000.vidconsole0                                            
 video         1  [ + ]   rk3399_vop            |-- vop@ff900000                                                            
 vidconsole    1  [ + ]   vidconsole0           |   `-- vop@ff900000.vidconsole0                                            
 video_brid    0  [ + ]   dw-mipi-dsi-rockchip  |-- dsi@ff960000                                                            
 dsi_host      0  [ + ]   dw_mipi_dsi           |   |-- dsihost                                                             
 panel         0  [ + ]   rm68200_panel         |   `-- panel@0                                                             
 pinctrl       0  [ + ]   rockchip_rk3399_pinc  |-- pinctrl     

 U-Boot 2024.10-armbian-2024.10-Sf919-Pbfb2-H3d34-Vade7-Bb703-R448a-dirty (Mar 03 2025 - 06:16:04 +0300)                     
                                                                                                                            
SoC: Rockchip rk3399                                                                                                        
Reset cause: RST                                                                                                            
Model: Orange Pi 4 LTS Board                                                                                                
DRAM:  4 GiB (effective 3.9 GiB)                                                                                            
PMIC:  RK808                                                                                                                
Core:  285 devices, 32 uclasses, devicetree: separate                                                                       
MMC:   mmc@fe320000: 1, mmc@fe330000: 0                                                                                     
Loading Environment from MMC... Reading from MMC(0)... *** Warning - bad CRC, using default environment                     
                                                                                                                            
rm68200_panel_of_to_plat() rm68200_panel panel@0: reset pin set done                                                        
 rm68200_panel_probe() rm68200_panel panel@0: Initializing MIPI DSI Panel Driver                                            
 rm68200_panel_probe() rm68200_panel panel@0: Reset sequence done                                                           
 rm68200_panel_probe() rm68200_panel panel@0: dsi set 4 lane, video-burst, RGB 24-bit mode                                  
rm68200_panel_enable_backlight() rm68200_panel panel@0: Panel set backlight func(), initializing panel                      
rm68200_panel_enable_backlight() rm68200_panel panel@0: attaching mipi dsi panel                                            
rm68200_panel_enable_backlight() rm68200_panel panel@0: perform panel initialization sequence                               
rm68200_panel_enable_backlight() rm68200_panel panel@0: panel initialization sequence done, panel probed!                   
rm68200_panel_enable_backlight() rm68200_panel panel@0: Panel set backlight func(), initializing panel                      
rm68200_panel_enable_backlight() rm68200_panel panel@0: attaching mipi dsi panel                                            
rm68200_panel_enable_backlight() rm68200_panel panel@0: perform panel initialization sequence                               
rm68200_panel_enable_backlight() rm68200_panel panel@0: panel initialization sequence done, panel probed!                   
In:    serial@ff1a0000                                                                                                      
Out:   serial@ff1a0000                                                                                                      
Err:   serial@ff1a0000                                                                                                      
Model: Orange Pi 4 LTS Board                                                                                                
Net:   Could not get PHY for ethernet@fe300000: addr 1                                                                      
      eth_initialize() No ethernet found.                                                                                   
                                                                                                                            
                                                                                                                            
Hit any key to stop autoboot:  0                                                                                            
=> dcdcddcdcdcdc<INTERRUPT>                                                                                                 
=> setenv splashimage 0x82000000                                                                                            
=> setenv splashfile /boot/splash.bmp                                                                                       
=> load mmc 0:1 ${splashimage} ${splashfile}                                                                                
1920138 bytes read in 61 ms (30 MiB/s)                                                                                      
=> bmp display ${splashimage}                                                                                               
=> <INTERRUPT>           



uclass_get_device_by_ofnode() Looking for clock-controller@ff760000                                                         
uclass_find_device_by_ofnode() Looking for clock-controller@ff760000                                                        
uclass_find_device_by_ofnode()       - checking reset                                                                       
uclass_find_device_by_ofnode()       - checking reset                                                                       
uclass_find_device_by_ofnode()    - result for clock-controller@ff760000: reset (ret=0)                                     
uclass_get_device_by_ofnode()    - result for clock-controller@ff760000: reset (ret=0)                                      
rm68200_panel_enable_backlight() rm68200_panel panel@0: Panel set backlight func(), initializing panel                      
rm68200_panel_enable_backlight() rm68200_panel panel@0: attaching mipi dsi panel                                            
rm68200_panel_enable_backlight() rm68200_panel panel@0: perform panel initialization sequence                               
rm68200_panel_enable_backlight() rm68200_panel panel@0: panel initialization sequence done, panel probed!                   
dsi_phy_post_set_mode() dw-mipi-dsi-rockchip dsi@ff960000: Set mode 00000000f4f3b4a0 enable 1                               
  device_bind_common() Bound device vop@ff900000.vidconsole0 to vop@ff900000                                                
      notify_dynamic() Sending event 5/(unknown) to spy 'efi_disk add'                                                      
      notify_dynamic() Sending event 5/(unknown) to spy 'efi_disk add'                                                      
In:    serial@ff1a0000                                                                                                      
Out:   serial@ff1a0000                                                                                                      
Err:   serial@ff1a0000                                                                                                      
Model: Orange Pi 4 LTS Board  
=> 
