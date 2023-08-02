| script                       | what to do                                                   | script type             |
| ---------------------------- | ------------------------------------------------------------ | ----------------------- |
| clean_all_software.sh        | Remove all software of Balldi. You need make  them again     | make/clean software use |
| make_all_software.sh         | Make all software of Balldi                                  | make/clean software use |
| kill_balldi_process.sh       | Kill all processes of Balldi's software                      | normal processes use    |
| start_balldi_process.sh      | Run all processes of Balldi if softwares have already been made | normal processes use    |
| systemd/add_systemd.sh       | Register all software of Balldi to systemctl services        | systemctl services use  |
| systemd/start_all_service.sh | Start all services of Balldi if you have already registered them | systemctl services use  |
| systemd/stop_all_service.sh  | Stop all services of Balldi if you have already registered them | systemctl services use  |

You shouldn't use 'normal processes' and 'systemctl services' together, choose one of them to use.