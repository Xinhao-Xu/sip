open -u tester,1 114.32.153.15
set ftp:passive-mode on
set ftp:list-options -a
mkdir test
cd test
put ./record/NotUpload/88_201222_162211.txt
