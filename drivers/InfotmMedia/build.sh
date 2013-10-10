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
	echo "error: "$1" failed "$2"!"
}

copy_file()
{
	cp $1 $2
	if [ $? -ne 0 ] ; then
		print_err "copy file" $1
		exit 1
	fi
}

### check the module state : $1 = "$dir" , $2 = "$str"
check_state()
{
	dir=$1
	str=$2
	if [ $# -ne 2 ]; then
		echo "error: check state parameters error!" 
		exit 1
	fi
	if [ ! -f $state_file ] ; then 
		touch $state_file
		echo "### stores the module enviroment state for scripts to check ###" >> $state_file
		echo "###  Do not modify it ###" >> $state_file
		echo "" >> $state_file
	fi
	
	line=`grep $str $state_file`
	if [ "$line" = "" ]; then
		echo $str" = 0" >> $state_file
		curr_state=0
	else 
		curr_state=`echo $line | awk '{print $NF}'`
	fi
	if [ $curr_state -eq 1 ] 
	then
		echo "warn: $dir has already built, would't do it more than once."
		continue
	fi
	return $curr_state
}

### compiler module enviroment build
comp_build()
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
				touch $state_file
				echo "### stores the module enviroment state for scripts to check ###" >> $state_file
				echo "###  Do not modify it ###" >> $state_file
				echo "" >> $state_file
			fi
			str="IM_"$dir"_state"
			line=`grep $str $state_file`
			if [ "$line" = "" ]; then
				echo $str" = 0" >> $state_file
				curr_state=0
			else 
				curr_state=`echo $line | awk '{print $NF}'`
			fi
			
			if [ $curr_state -eq 1 ] ; then
				echo "warn: $dir has already built, would't do it more than once."
				continue
			fi
			
			## execute the script if needed.
			cd $dir
			if [ -f buildenv.sh ] ; then
				./buildenv.sh 
				if [ $? -ne 0 ]; then
					print_err $dir "buildenv"
				fi
			fi
			cd - 1>/dev/null
			sed -i "s/$str = 0/$str = 1/g" $state_file
		fi
	done
}

###  real external module 
ext_change_file_list_build()
{
	if [ ! -f $ext_file_list ] ; then
		return 0
	fi
	exec 101<> $ext_file_list
	cnt=0
	while read line <&101
	do {
		((cnt ++))
		len=`echo $line | awk '{print length()}'`
		if [ $len -eq 0 ] || [ "$line" != "${line/"#"}" ];then
			continue
		else
			file=$line
		fi
		
		if [ -f $DRV_ROOT/$file ]; then
			copy_file $DRV_ROOT/$file $WP_PATH/$file.tmpbak
		fi
		copy_file $WP_PATH/$file $DRV_ROOT/$file
	}
	done
	exec 101>&-
}

### external module enviroment build
### should check the state first, then call the truely dealing function
ext_build()
{
	for dir in `ls -l $IM_EXTERNAL_ROOT | grep ^d | awk '{print $NF}'`
	do
		if [ $1 = "all" ] || [ $dir = $1 ]; then
			if [ ! -f $state_file ] ; then 
				touch $state_file
				echo "### stores the module enviroment state for scripts to check ###" >> $state_file
				echo "###  Do not modify it ###" >> $state_file
				echo "" >> $state_file
			fi
			str="IM_EXTERNAL_"$dir"_state"
			line=`grep $str $state_file`
			if [ "$line" = "" ]; then
				echo $str" = 0" >> $state_file
				curr_state=0
			else 
				curr_state=`echo $line | awk '{print $NF}'`
			fi

			if [ $curr_state -eq 1 ] ; then
				echo "warn: ${dir} has already built, would't do it more than once.."
				continue
			fi

			cd $IM_EXTERNAL_ROOT/$dir

			if [ -f buildenv.sh ]
			then
				echo "execute " $dir "buildenv.sh"
				./buildenv.sh
				if [ $? -ne 0 ]
				then
					print_err "external buildenv.sh" $dir
					exit 1
				fi
			fi

			#echo " external build: " $dir
		   	#ext_change_file_list_build
			cd - 1>/dev/null
			sed -i "s/$str = 0/$str = 1/g" $state_file
		fi
	done
}


############################################################

## no params, build all the enviroment
if [ $# -eq 0 ] ; then 
	echo
    	comp_build all
	echo "##########compiler buildenv all success##########"
	ext_build all
	echo "##########external buildenv all success##########"
	exit 0
fi

exit 0




