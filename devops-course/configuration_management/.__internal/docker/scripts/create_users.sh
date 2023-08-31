#!/bin/bash
set -e

HOME_DIR=/data
__UC=${USERS_COUNT:-1}
__UNP=${USER_NAME_PREFIX:-red}

create_user() {
    user=$1
    uid=$2
    group=$3
    gid=$4

    mkdir -p ${HOME_DIR}/${user}
    groupadd -g ${gid} ${group}
    useradd -m -s /bin/bash -u ${uid} -g ${gid} -G ${group} ${user}
    chown -R ${user} "${HOME_DIR}/${user}" && chgrp -R ${group} "${HOME_DIR}/${user}"
    echo "$user" | passwd "$user" --stdin
}

repeat_zero() {
    count=$1
    printf '0%.0s' {1..$count}
}

length_users=${#__UC}
echo ${length_users} ${__UC}
for u in $(seq 0 $__UC); do
    len_u=${#u}
    _username=${__UNP}${u}
    _expr=$(($length_users - $len_u))
    _full_count=${u}
    if [ ${_expr} -ge 1 ]; then
        _full_count=$(repeat_zero ${_expr})${u}
        _username=${__UNP}${_full_count}
    fi
    echo create user ${_username}
    create_user $_username 54${_full_count} $_username 54${_full_count}
done
