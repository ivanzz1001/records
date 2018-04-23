#!/bin/sh

access_token="24.49995be00ce4c66e5c8434af2f5a20cf.2592000.1527063479.282335-10887708"


pic_name=bank-card8.jpg
pic_base64=`base64 -w 0 $pic_name > bank-card.base64`
pic_encodeurl=`./urlencode bank-card.base64`

echo "image=$pic_encodeurl" > bankcard_data

echo "curl -i -k 'https://aip.baidubce.com/rest/2.0/ocr/v1/bankcard?access_token=$access_token' --data @bankcard_data -H 'Content-Type:application/x-www-form-urlencoded'"
