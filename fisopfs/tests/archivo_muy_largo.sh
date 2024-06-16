#!/bin/bash

VERDE="\e[0;32m"
ROJO="\e[0;31m"
RESET="\e[0m"

cd prueba

LOREM_IPSUM="Lorem ipsum dolor sit amet, consectetur adipiscing elit. Vestibulum sed tristique nibh. Donec vel tempor nibh. Fusce congue lacus ligula, at mollis erat bibendum ut. Morbi facilisis fermentum diam eu consectetur. Sed ac dignissim neque. Nam faucibus bibendum ex posuere dapibus. Nunc varius faucibus ligula et aliquam. Nam efficitur turpis est, eget dignissim nulla ornare id. Vivamus tincidunt, lectus id cursus hendrerit, magna nulla porta ex, in molestie leo purus sit amet ipsum. Pellentesque blandit, ex eu scelerisque accumsan, odio sapien placerat massa, ut tincidunt dolor elit a lorem. Suspendisse non congue urna. Vivamus vulputate lacus non posuere rhoncus. Maecenas varius hendrerit fringilla. Vivamus in enim massa. Vivamus a sem in elit dapibus pulvinar in ac urna. Pellentesque habitant morbi tristique senectus et netus et malesuada fames ac turpis egestas. Nam sit amet purus non nunc congue eleifend in malesuada mauris. Etiam in mattis lacus. Donec ante quam, vestibulum id tempor vitae, luctus lacinia enim. Pellentesque iaculis ut purus in sollicitudin. Quisque at sapien semper, vulputate augue id, egestas turpis. Aliquam elementum elit in varius ultricies. Vivamus accumsan libero sollicitudin lectus pulvinar, nec vestibulum arcu aliquam. Aenean tincidunt malesuada molestie. Quisque dapibus odio id massa lobortis, eu finibus diam rhoncus. Integer molestie porta dui a ornare. Nullam posuere odio at justo lobortis, a placerat odio placerat. Integer eros ipsum, fermentum ac nulla pharetra, mattis viverra quam. Integer id enim eget tellus congue pretium. Integer vitae efficitur mi. Aliquam pharetra non dui nec rhoncus. Curabitur eu nulla interdum nunc imperdiet consequat vel id lorem. Suspendisse quis ante nec risus tristique tincidunt. Integer sed consectetur neque. Aenean neque nulla, suscipit vitae condimentum sit amet, accumsan sit amet velit. Sed quis velit nibh. Etiam egestas est vel ante posuere, sed iaculis orci viverra. Nullam efficitur sem nibh, non ultricies purus iaculis vel. Donec fermentum libero ac varius imperdiet. Nullam tellus leo, auctor eget ullamcorper et, pharetra maximus lorem. Donec finibus in dui cursus mattis. Donec ultrices velit vel ultricies ullamcorper. Nullam faucibus vehicula purus. Etiam rutrum turpis vitae dolor tristique aliquet. Aenean euismod nunc velit, at mattis tortor ultrices vel. Aenean volutpat libero eget felis dignissim placerat. Nam pharetra varius euismod. Pellentesque fermentum scelerisque arcu et faucibus. Proin justo eros, tincidunt et magna ac, porttitor viverra lacus. Nam sem libero, lacinia ut lobortis ac, luctus sed libero. Nunc et faucibus sem, in malesuada odio. Cras convallis ac tortor vel interdum. Donec sed posuere urna. Mauris ultrices ultrices quam, quis tristique nisi placerat id. Vestibulum ut diam viverra, tristique neque sed, vulputate neque. Aliquam euismod vel justo vel aliquet." 

echo $LOREM_IPSUM > archivo_largo.txt 2> /dev/null

RESULTADO=$(( $RESULTADO + $? ))

if grep -q "$LOREM_IPSUM" archivo_largo.txt; then
    echo -e "Creación de archivos largos: $VERDE OK $RESET"
    rm archivo_largo.txt
    exit 0
else
    echo -e "Creación de archivos largos: $ROJO FAIL $RESET"
    rm archivo_largo.txt
    exit 1
fi