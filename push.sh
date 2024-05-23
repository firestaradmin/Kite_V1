#!/bin/bash
versionS="v2.0"

echo "========================================"
echo "自动添加提交commit当前分支并推送到远端仓库-懒人的脚本"
echo "Version $versionS @firestaradmin"
echo "========================================"


	
sysinfo=$HOSTNAME"_"$(uname -m)

echo '正在获取仓库最新状态...'
git fetch
echo '获取仓库最新状态完成'

git_status_output=$(git status --porcelain)  

if [[ -n "$git_status_output" ]]; then  
    # 如果输出不为空，表示有变动  
    has_changes=true  
else  
    # 如果输出为空，表示没有变动  
    has_changes=false  
fi  

# files changed:
files_changed_list=$(git status --porcelain | awk '{print $2}')

if [ $has_changes == true ]; then  
    # echo 'yes'
    echo '检测到有文件变动，自动接受所有变动'
    git add .
    # git status --porcelain | awk '{print $2}'
else  
    echo '没有需要添加的文件变动'
fi  


if [ $has_changes == true ]; then  

    # 初始化变量来存储不同状态的文件列表  
    modified_files=""  
    added_files=""  
    deleted_files=""  
    renamed_files=""  
    unmerged_files=""  
    
    commit_string=""
    # 使用git status的--short选项获取状态输出  
    git_status_output=$(git status --short)  
    # 遍历输出，分类处理文件状态  
    while IFS= read -r line; do  
        # 提取状态字符和文件名  
        status_char=${line:0:1}  
        filename=${line#?? } # 注意：这里可能需要调整空格数量，取决于git status的输出  
        # echo $status_char
        # 根据状态字符进行分类  
        case "$status_char" in  
            M|m)  # 修改的文件  
                modified_files+="$filename  
    "  
                ;;  
            A|a)  # 新增的文件  
                added_files+="$filename  
    "  
                ;;  
            D|d)  # 删除的文件  
                deleted_files+="$filename  
    "  
                ;;  
            R|r)  # 重命名的文件（这里简化处理，只打印源文件名）  
                renamed_files+="$filename  
    "  
                ;;  
            U|u)  # 更新但未合并的文件  
                unmerged_files+="$filename (CONFLICTED)  
    "  
                ;;  
            # 可以根据需要添加其他状态字符的处理  
            # ...  
            *)    # 其他未知状态  
                echo "Unknown status for file: $filename"  
                ;;  
        esac  
    done <<< "$git_status_output"  

    # 打印分类后的文件列表  
    # echo -e用于解释转义字符（比如换行符\n），这样文件列表会以多行形式打印出来。
    if [[ -n "$modified_files" ]]; then  
        echo "==== Modified files:"  
        echo -e "    $modified_files"  
    fi  
    if [[ -n "$added_files" ]]; then  
    echo "==== Added files:"  
        echo -e "    $added_files"  
    fi  
    if [[ -n "$deleted_files" ]]; then  
    echo "==== Deleted files:" 
        echo -e "    $deleted_files"   
    fi  
    if [[ -n "$renamed_files" ]]; then  
    echo "==== Renamed files:"  
        echo -e "    $renamed_files"  
    fi  
    if [[ -n "$unmerged_files" ]]; then  
        echo "==== Unmerged files (CONFLICTED):"  
        echo -e "    $unmerged_files"  
    fi  

    echo "请检查以上文件列表，如果没问题请输入commit 信息，或者直接回车自动生成commit，如果有问题请直接退出脚本"
	read commitS
	if [ ! "$commitS" ]; then
		# default opreation
		commit_string="$(date) with push.sh $versionS from $sysinfo"
		# echo "$(date) with push.sh"
	else
		commit_string="$commitS | with push.sh $versionS from $sysinfo"
		# echo "$commitS | with push.sh"
	fi
    # echo "commit 信息：$commit_string"
    git commit -m "$commit_string"


fi  

judges=$(git status | grep 'is ahead of')
# echo "$judges"

if [[ -n "$judges" ]]; then
    echo  
    echo  
    echo "-----------------------------------"
    echo "检测到领先远端分支，按任意键 PUSH >>>"  
    read
    git push 
else  
    echo '当前分支版本与远端分支相同'
fi


echo  
echo "-----------------------------------"
echo -e $(git status)
echo "-----------------------------------"
echo "按任意键退出..."  
read











# 输出系统信息测试
if [ 0 == 1 ] ;then
	
	echo "主机名  ： $HOSTNAME"
	echo "系统版本： `uname`"
	echo "内核版本： `uname -r`"
	echo "CPU型号 ：`cat /proc/cpuinfo | grep "model name"|cut -d: -f2|head -1`"
	echo "内存大小： `free -m |grep Mem|tr -s " "|cut -d" " -f2` MB"
	echo -e "硬盘分区：\n`df -hT |egrep -o "^/dev/sd.*\>"|tr -s " " | cut -d" " -f1,3|sort`"


	release_num=$(lsb_release -r --short)
	code_name=$(lsb_release -c --short)
	hw_arch=$(uname -m)
	echo "release_num= $release_num"
	echo "code_name= $code_name"
	echo "hardware Architecture = $hw_arch"
	#output result:
	#release_num= 22.04
	#code_name= jammy
	#hardware Architecture = x86_64
	exit
fi