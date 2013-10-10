#!/bin/bash
###  sam_ye, 2012/06/02  ###

### to fix the current path
LOCAL_PATH=`pwd`

state_file="env_state_must_not_delete"
IM_EXTERNAL_ROOT=$LOCAL_PATH/external/project/
DRV_ROOT="../../../../../"
WP_PATH="../../workcopy/"
ext_file_list="change_file_list.txt"
	

###########################################################

### error print 
print_err()
{
	echo "error: " $1 " failed " $2 "!"
}

copy_file()
{
	cp $1 $2
	if [ $? -ne 0 ] ; then
		print_err "copy file " $1
		exit 1
	fi
}

move_file()
{
	mv $1 $2
	if [ $? -ne 0 ] ; then
		print_err "move file " $1
		exit 1
	fi
}

###  user help
print_help()
{
	echo "####################################################################################"
	echo "$0 [help/module/external_module]"
	echo "	collect the indicated module envirionment, if not set, all the modules include external_module will collect."
	echo "	module: optional, module folder."
	echo "	external_module: optionanl, external module folder."
	echo "example:"
	echo "	$0"
	echo "	$0 foundations"
	echo "	$0 external_ids_drv_i800"
	echo "###################################################################################"
}

### check the module state : $1 = "$dir", $2 = "$str"
check_state()
{
	dir=$1
	str=$2
	if [ $# -ne 2 ]; then
		echo "error: check state parameters error " 
		exit 1
	fi
	if [ ! -f $state_file ] ; then 
		echo "warn: $dir not built yet , check it"
		return 1
	fi
	
	line=`grep $str $state_file`
	if [ "$line" = "" ]; then
		echo "warn: $dir not built yet , check it"
		return 1
	else 
		curr_state=`echo $line | awk '{print $NF}'`
	fi
	if [ $curr_state -eq 0 ] ; then
		echo "warn: $dir already collected , please check it"
		return 1
	fi
	return 0
}

### compiler module enviroment build
comp_collect()
{
	exclude=".svn"
	exclude1="include"
	exclude2="packages"
	exclude3="external"

	for dir in `ls -l | grep ^d | awk '{print $NF}'`
	do
		if [ "$dir" = $exclude1 ] || [ "$dir" = $exclude2 ] || [ "$dir" = $exclude3 ] ;then
			continue
		fi

		if [ $1 = "all" ] || [ $dir = $1 ]; then
			### check the state
			if [ ! -f $state_file ] ; then 
				echo "warn: $dir not built yet , check it"
				continue
			fi
			str="IM_"$dir"_state"
			line=`grep $str $state_file`
			if [ "$line" = "" ]; then
				echo "warn: $dir not built yet , check it"
				continue
			else 
				curr_state=`echo $line | awk '{print $NF}'`
			fi
			
			if [ $curr_state -eq 0 ] ; then
				echo "warn: $dir already collected , please check it"
				continue
			fi
			
			## execute the script if needed.
			cd $dir
			if [ -f collectenv.sh ] ; then
				./collectenv.sh 
				if [ $? -ne 0 ]; then
					print_err $dir " collectenv"
				fi
			fi
			cd - 1>/dev/null

			## collect file 
			sed -i "s/$str = 1/$str = 0/g" $state_file
		fi
	done
}

###  real external module 
ext_change_file_list_collect()
{
	if [ ! -f $ext_file_list ] ; then
		return 0
	fi
	exec 102<> $ext_file_list
	while read line <&102
	do {
		len=`echo $line | awk '{print length()}'`
		if [ $len -eq 0 ] || [ "$line" != "${line/"#"}" ];then
			continue
		else
			file=$line
		fi

		move_file $DRV_ROOT/$file $WP_PATH/$file
		if [ -f $WP_PATH/$file.tmpbak ] ; then
			move_file $WP_PATH/$file.tmpbak $DRV_ROOT/$file 
		fi
	}
	done
	exec 102>&-
}

### external module enviroment build
### should check the state first, then call the truely dealing function
ext_collect()
{
	for dir in `ls -l $IM_EXTERNAL_ROOT | grep ^d | awk '{print $NF}'`
	do
		if [ $1 = "all" ] || [ $dir = $1 ]; then
			if [ ! -f $state_file ] ; then
				echo $dir " not built yet , check it"
				continue
			fi
			str="IM_EXTERNAL_"$dir"_state"
			line=`grep $str $state_file`
			if [ "$line" = "" ]; then
				echo "warn: $dir not built yet , check it"
				continue
			else 
				curr_state=`echo $line | awk '{print $NF}'`
			fi

			if [ $curr_state -eq 0 ] ; then
				echo "warn: ${dir} already collected, please check it"
				continue
			fi

			cd $IM_EXTERNAL_ROOT/$dir

			if [ -f collectenv.sh ]
			then
				echo "execute " $dir "collectenv.sh"
				./collectenv.sh
				if [ $? -ne 0 ]
				then
					print_err "external collectenv.sh " $dir
					exit 1
				fi
			fi

			#echo "external collect: " $dir
		   	#ext_change_file_list_collect
			cd - 1>/dev/null
			sed -i "s/$str = 1/$str = 0/g" $state_file
		fi
	done
}


############################################################

if [ $# -eq 1 ] && [ $1 = "help" ] ; then 
	print_help
	exit 0
fi

## no params, build all the enviroment
if [ $# -eq 0 ] ; then 
	comp_collect all
	echo "##########compiler buildenv all success##########"
	echo
	ext_collect all
	echo "##########external buildenv all success##########"
	exit 0
fi

### to looply deal the arguments.
exit 0



