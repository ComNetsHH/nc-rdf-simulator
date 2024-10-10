#scp -r ckf1377@hummel.rz.tu-harburg.de:/work/et6/ckf1377/uav_flooding_results/v22_parsed res/v22_parsed
rsync --include='summary*' --exclude='*' --progress -avzhe ssh ckf1377@hummel.rz.tu-harburg.de:/work/et6/ckf1377/uav_flooding_results/v27_parsed/ res/v27_parsed

rsync --include='sf_n340_i360*' --exclude='*' --progress -avzhe ssh ckf1377@hummel.rz.tu-harburg.de:/work/et6/ckf1377/uav_flooding_results/v27_parsed/ res/v27_parsed

rsync --exclude='*.dat' --include='summary*' --exclude='*' --progress -avzhe 'ssh -p 2222' ckf1377@localhost:/work/et6/ckf1377/uav_flooding_results/v38_parsed/ res/v38_parsed

rsync --progress --include='summary*' --exclude='*' -avzhe 'ssh -p 2222' ckf1377@localhost:/work/et6/ckf1377/uav_flooding_results/v501_parsed/ res/v501_parsed

scp -r -P 2222 ckf1377@localhost:/work/et6/ckf1377/uav_flooding_results/v501_parsed/ res/v501_parsed

rsync --progress -avzhe 'ssh -p 2222' ckf1377@localhost:/work/et6/ckf1377/uav_flooding_results/v601/ res/v601