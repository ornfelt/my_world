#!/bin/sh

JKSSL_BIN=./jkssl
OPSSL_BIN=openssl

test_4k=`mktemp /tmp/jkssl_zero_4k_XXX`
test_1M=`mktemp /tmp/jkssl_zero_1M_XXX`
test_16M=`mktemp /tmp/jkssl_zero_16M_XXX`

dd if=/dev/zero of=$test_4k bs=1024 count=4 > /dev/null 2> /dev/null
dd if=/dev/zero of=$test_1M bs=1024 count=1024 > /dev/null 2> /dev/null
dd if=/dev/zero of=$test_16M bs=1024 count=16384 > /dev/null 2> /dev/null

print_result_ok()
{
	printf "\e[0m[\e[1;32mOK\e[0m] $1\e[0m\n"
}

print_result_ko()
{
	printf "\e[0m[\e[1;31mKO\e[0m] $1\e[0m\n"
}

print_result()
{
	if [ "$2" = "$3" ]
	then
		print_result_ok "$1"
	else
		print_result_ko "$1"
	fi
}

test_dgst_do()
{
	ret_jkssl=`$JKSSL_BIN dgst -$2 $4 $3 | cut -d '=' -f 2`
	ret_opssl=`$OPSSL_BIN dgst -$2 $4 $3 | cut -d '=' -f 2`
	print_result "$1 $3" "$ret_jkssl" "$ret_opssl"
}

test_hash_do()
{
	test_dgst_do "$1" $1 $2 ""
	test_dgst_do "$1 hmac short" $1 $2 "-hmac cc"
	test_dgst_do "$1 hmac long" $1 $2 "-hmac cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc"
}

test_hash()
{
	test_hash_do $1 /dev/null
	test_hash_do $1 $test_4k
	test_hash_do $1 $test_16M
}

test_hash_all()
{
	test_hash md4
	echo
	test_hash md5
	echo
	test_hash sha1
	echo
	test_hash sha224
	echo
	test_hash sha256
	echo
	test_hash sha384
	echo
	test_hash sha512
	echo
	test_hash ripemd160
	echo
	test_hash sm3
}

test_base64_encode()
{
	ret_jkssl=`$JKSSL_BIN base64 -in $1 | $OPSSL_BIN sha1 | cut -d ' ' -f2`
	ret_opssl=`$OPSSL_BIN base64 -in $1 | $OPSSL_BIN sha1 | cut -d ' ' -f2`
	print_result "base64 encode $1" "$ret_jkssl" "$ret_opssl"
}

test_base64_decode()
{
	file=`mktemp`
	cat $1 | openssl base64 > $file
	ret_jkssl=`$JKSSL_BIN base64 -d -in $file | $OPSSL_BIN sha1 | cut -d ' ' -f2`
	ret_opssl=`$OPSSL_BIN base64 -d -in $file | $OPSSL_BIN sha1 | cut -d ' ' -f2`
	rm $file
	print_result "base64 decode $1" "$ret_jkssl" "$ret_opssl"
}

test_base64()
{
	test_base64_encode /dev/null
	test_base64_encode $test_4k
	test_base64_encode $test_1M
	test_base64_decode /dev/null
	test_base64_decode $test_4k
	test_base64_decode $test_1M
}

test_base64_all()
{
	test_base64
}

test_cipher_encrypt()
{
	iv="8877665544332211"
	ret_jkssl=`$JKSSL_BIN enc -$1 -a -e -K $2 -iv $iv -in $3 2>&- | $OPSSL_BIN sha1 | cut -d ' ' -f2`
	ret_opssl=`$OPSSL_BIN enc -$1 -a -e -K $2 -iv $iv -in $3 2>&- | $OPSSL_BIN sha1 | cut -d ' ' -f2`
	print_result "$1 encrypt $3" "$ret_jkssl" "$ret_opssl"
}

test_cipher_decrypt()
{
	iv="8877665544332211"
	file=`mktemp`
	cat $3 | openssl enc -$1 -a -e -K $2 -iv $iv > $file 2>&-
	ret_jkssl=`$JKSSL_BIN enc -$1 -a -d -K $2 -iv $iv -in $file 2>&- | $OPSSL_BIN sha1 | cut -d ' ' -f2`
	ret_opssl=`$OPSSL_BIN enc -$1 -a -d -K $2 -iv $iv -in $file 2>&- | $OPSSL_BIN sha1 | cut -d ' ' -f2`
	rm $file
	print_result "$1 decrypt $3" "$ret_jkssl" "$ret_opssl"
}

test_cipher()
{
	test_cipher_encrypt $1 $2 /dev/null
	test_cipher_encrypt $1 $2 $test_4k
	test_cipher_encrypt $1 $2 $test_1M
	test_cipher_decrypt $1 $2 /dev/null
	test_cipher_decrypt $1 $2 $test_4k
	test_cipher_decrypt $1 $2 $test_1M
}

test_cipher_part()
{
	test_cipher $1-ecb $2
	echo
	test_cipher $1-cbc $2
	echo
	test_cipher $1-cfb $2
	echo
	test_cipher $1-ofb $2
}

test_des1()
{
	key="0123456789abcdef"
	test_cipher_part des $key
}

test_des2()
{
	key="0123456789abcdef1122334455667788"
	test_cipher_part des-ede $key
}

test_des3()
{
	key="0123456789abcdef1122334455667788fedcba9876543210"
	test_cipher_part des-ede3 $key
}

test_des()
{
	test_des1
	echo
	test_des2
	echo
	test_des3
}

test_aes128()
{
	key="0123456789abcdef1122334455667788"
	test_cipher_part aes-128 $key
}

test_aes192()
{
	key="0123456789abcdef1122334455667788fedcba9876543210"
	test_cipher_part aes-192 $key
}

test_aes256()
{
	key="0123456789abcdef1122334455667788fedcba98765432108877665544332211"
	test_cipher_part aes-256 $key
}

test_aes()
{
	test_aes128
	echo
	test_aes192
	echo
	test_aes256
}

test_camellia128()
{
	key="0123456789abcdef1122334455667788"
	test_cipher_part camellia-128 $key
}

test_camellia192()
{
	key="0123456789abcdef1122334455667788fedcba9876543210"
	test_cipher_part camellia-192 $key
}

test_camellia256()
{
	key="0123456789abcdef1122334455667788fedcba98765432108877665544332211"
	test_cipher_part camellia-256 $key
}

test_camellia()
{
	test_camellia128
	echo
	test_camellia192
	echo
	test_camellia256
}

test_aria128()
{
	key="0123456789abcdef1122334455667788"
	test_cipher_part aria-128 $key
}

test_aria192()
{
	key="0123456789abcdef1122334455667788fedcba9876543210"
	test_cipher_part aria-192 $key
}

test_aria256()
{
	key="0123456789abcdef1122334455667788fedcba98765432108877665544332211"
	test_cipher_part aria-256 $key
}

test_aria()
{
	test_aria128
	echo
	test_aria192
	echo
	test_aria256
}

test_chacha20()
{
	key="0123456789abcdef1122334455667788fedcba98765432108877665544332211"
	test_cipher chacha20 $key
}

test_rc4()
{
	key="0123456789abcdef1122334455667788"
	test_cipher rc4 $key
}

test_rc2()
{
	key="0123456789abcdef1122334455667788"
	test_cipher_part rc2 $key
}

test_seed()
{
	key="0123456789abcdef1122334455667788"
	test_cipher_part seed $key
}

test_bf()
{
	key="0123456789abcdef1122334455667788"
	test_cipher_part bf $key
}

test_cast5()
{
	key="0123456789abcdef1122334455667788"
	test_cipher_part cast5 $key
}

test_sm4()
{
	key="0123456789abcdef1122334455667788fedcba9876543210"
	test_cipher_part sm4 $key
}

test_enc()
{
	ret_jkssl=`$JKSSL_BIN enc -aes-128-cbc -k test1 -S a1 -P 2>&- | grep key=`
	ret_opssl=`$OPSSL_BIN enc -aes-128-cbc -k test1 -S a1 -P 2>&- | grep key=`
	print_result "enc_kS_key_short" $ret_jkssl $ret_opssl
	ret_jkssl=`$JKSSL_BIN enc -aes-128-cbc -k test1 -S a1 -P 2>&- | grep iv=`
	ret_opssl=`$OPSSL_BIN enc -aes-128-cbc -k test1 -S a1 -P 2>&- | grep iv=`
	print_result "enc_kS_iv_short" $ret_jkssl $ret_opssl
	ret_jkssl=`$JKSSL_BIN enc -aes-128-cbc -k test1test2test3test4test5test6test7test8test9testatestbtestctestetestf -S a1a2a3a4a5a6a7a8a9aaabacadaeafb0 -P 2>&- | grep key=`
	ret_opssl=`$OPSSL_BIN enc -aes-128-cbc -k test1test2test3test4test5test6test7test8test9testatestbtestctestetestf -S a1a2a3a4a5a6a7a8a9aaabacadaeafb0 -P 2>&- | grep key=`
	print_result "enc_kS_key_long" $ret_jkssl $ret_opssl
	ret_jkssl=`$JKSSL_BIN enc -aes-128-cbc -k test1test2test3test4test5test6test7test8test9testatestbtestctestetestf -S a1a2a3a4a5a6a7a8a9aaabacadaeafb0 -P 2>&- | grep iv=`
	ret_opssl=`$OPSSL_BIN enc -aes-128-cbc -k test1test2test3test4test5test6test7test8test9testatestbtestctestetestf -S a1a2a3a4a5a6a7a8a9aaabacadaeafb0 -P 2>&- | grep iv=`
	print_result "enc_kS_iv_long" $ret_jkssl $ret_opssl
	ret_jkssl=`$JKSSL_BIN enc -aes-128-cbc -k test1 -S a1 -P -md sha1 2>&- | grep key=`
	ret_opssl=`$OPSSL_BIN enc -aes-128-cbc -k test1 -S a1 -P -md sha1 2>&- | grep key=`
	print_result "enc_kS_sha1" $ret_jkssl $ret_opssl
	ret_jkssl=`$JKSSL_BIN enc -aes-128-cbc -k test1 -S a1 -P -md sha512 2>&- | grep key=`
	ret_opssl=`$OPSSL_BIN enc -aes-128-cbc -k test1 -S a1 -P -md sha512 2>&- | grep key=`
	print_result "enc_kS_sha512" $ret_jkssl $ret_opssl

	ret_jkssl=`$JKSSL_BIN enc -aes-128-cbc -k test1 -S a1 -P -pbkdf2 2>&- | grep key=`
	ret_opssl=`$OPSSL_BIN enc -aes-128-cbc -k test1 -S a1 -P -pbkdf2 2>&- | grep key=`
	print_result "enc_kS_pbkdf2_key_short" $ret_jkssl $ret_opssl
	ret_jkssl=`$JKSSL_BIN enc -aes-128-cbc -k test1 -S a1 -P -pbkdf2 2>&- | grep iv=`
	ret_opssl=`$OPSSL_BIN enc -aes-128-cbc -k test1 -S a1 -P -pbkdf2 2>&- | grep iv=`
	print_result "enc_kS_pbkdf2_iv_short" $ret_jkssl $ret_opssl
	ret_jkssl=`$JKSSL_BIN enc -aes-128-cbc -k test1test2test3test4test5test6test7test8test9testatestbtestctestetestf -S a1a2a3a4a5a6a7a8a9aaabacadaeafb0 -P -pbkdf2 2>&- | grep key=`
	ret_opssl=`$OPSSL_BIN enc -aes-128-cbc -k test1test2test3test4test5test6test7test8test9testatestbtestctestetestf -S a1a2a3a4a5a6a7a8a9aaabacadaeafb0 -P -pbkdf2 2>&- | grep key=`
	print_result "enc_kS_pbkdf2_key_long" $ret_jkssl $ret_opssl
	ret_jkssl=`$JKSSL_BIN enc -aes-128-cbc -k test1test2test3test4test5test6test7test8test9testatestbtestctestetestf -S a1a2a3a4a5a6a7a8a9aaabacadaeafb0 -P -pbkdf2 2>&- | grep iv=`
	ret_opssl=`$OPSSL_BIN enc -aes-128-cbc -k test1test2test3test4test5test6test7test8test9testatestbtestctestetestf -S a1a2a3a4a5a6a7a8a9aaabacadaeafb0 -P -pbkdf2 2>&- | grep iv=`
	print_result "enc_kS_pbkdf2_iv_long" $ret_jkssl $ret_opssl
	ret_jkssl=`$JKSSL_BIN enc -aes-128-cbc -k test1 -S a1 -P -md sha1 -pbkdf2 2>&- | grep key=`
	ret_opssl=`$OPSSL_BIN enc -aes-128-cbc -k test1 -S a1 -P -md sha1 -pbkdf2 2>&- | grep key=`
	print_result "enc_kS_pbkdf2_sha1" $ret_jkssl $ret_opssl
	ret_jkssl=`$JKSSL_BIN enc -aes-128-cbc -k test1 -S a1 -P -md sha512 -pbkdf2 2>&- | grep key=`
	ret_opssl=`$OPSSL_BIN enc -aes-128-cbc -k test1 -S a1 -P -md sha512 -pbkdf2 2>&- | grep key=`
	print_result "enc_kS_pbkdf2_sha512" $ret_jkssl $ret_opssl
	ret_jkssl=`$JKSSL_BIN enc -aes-128-cbc -k test1 -S a1 -P -iter 50 2>&- | grep key=`
	ret_opssl=`$OPSSL_BIN enc -aes-128-cbc -k test1 -S a1 -P -iter 50 2>&- | grep key=`
	print_result "enc_kS_pbkdf2_iter" $ret_jkssl $ret_opssl
	ret_jkssl=`$JKSSL_BIN enc -aes-128-cbc -k test1 -S a1 -P -md sha512 -iter 500 2>&- | grep key=`
	ret_opssl=`$OPSSL_BIN enc -aes-128-cbc -k test1 -S a1 -P -md sha512 -iter 500 2>&- | grep key=`
	print_result "enc_kS_pbkdf2_iter_sha512" $ret_jkssl $ret_opssl
}

test_bignum()
{
	ret_jkssl=`$JKSSL_BIN bignum $2 "$3" $4`
	print_result "bignum_$1" $ret_jkssl $5
}

test_bignum_mod()
{
	ret_jkssl=`$JKSSL_BIN bignum $2 "$3" $4 $5`
	print_result "bignum_$1" $ret_jkssl $6
}

test_bignum_all()
{
	test_bignum add_simple 1 + 2 3
	test_bignum add_zero 0 + 0 0
	test_bignum add_one_zero 1 + 0 1
	test_bignum add_big_small 112233445566778899 + 10 112233445566778909
	test_bignum add_big_big 112233445566778899 + 112233445566778899 224466891133557798
	test_bignum add_huge_huge 2538687043280516217968757858774735713134894094727501826178087490310416 + 1269343521640258108984378929387367856567447047363750913089043745155208 3808030564920774326953136788162103569702341142091252739267131235465624
	test_bignum add_negative_png 5 + -2 3
	test_bignum add_negative_pnl 2 + -5 -3
	test_bignum add_negative_npg -6 + 4 -2
	test_bignum add_negative_npl -5 + 9 4
	test_bignum add_negative_nng -5 + -4 -9
	test_bignum add_negative_nnl -3 + -9 -12
	echo
	test_bignum sub_simple 3 - 2 1
	test_bignum sub_zero 0 - 0 0
	test_bignum sub_one_zero 1 - 0 1
	test_bignum sub_big_small 112233445566778899 - 10 112233445566778889
	test_bignum sub_big_big 112233445566778899 - 112233445566778889 10
	test_bignum sub_huge_huge 2538687043280516217968757858774735713134894094727501826178087490310416 - 1269343521640258108984378929387367856567447047363750913089043745155207 1269343521640258108984378929387367856567447047363750913089043745155209
	test_bignum sub_negative_png 5 - -2 7
	test_bignum sub_negative_pnl 3 - -7 10
	test_bignum sub_negative_npg -7 - 4 -11
	test_bignum sub_negative_npl -2 - 5 -7
	test_bignum sub_negative_nng -9 - -3 -6
	test_bignum sub_negative_nnl -4 - -9 5
	echo
	test_bignum mul_simple 3 _ 2 6
	test_bignum mul_zero 3 _ 0 0
	test_bignum mul_one 1 _ 8 8
	test_bignum mul_big_small 112233445566778899 _ 9 1010101010101010091
	test_bignum mul_big_big 112233445566778899 _ 112233445566778899 12596346303791122097392430351652201
	test_bignum mul_huge_huge 2538687043280516217968757858774735713134894094727501826178087490310416 _ 2538687043280516217968757858774735713134894094727501826178087490310316 6444931903720369624895988058295186490773351122708453192597035122359526207702594532528293432256556085010669425565620297661676703939007051456
	test_bignum mul_huge_huge 105449817522143713873289191736816315467970330682548964008145209069279544391796791572803429807023304312766876266272581399518725843020156573640514089230038703743453303966007461476603345363277518865734459405413382165767490114833793451131084417922288730672653166013004415494808153304408837782464780024258853874874741165611254153116498208168945386520576105449817522143713873289191736816315467970330682548964008145209069279544391796791572803429807023304312766876266272581399518725843020156573640514089230038703743453303966007461476603345363277518865734459405413382165767490114833793451131084417922288730672653166013004415494808153304408837782464780024258853874874741165611254153116498208168945386520576 _ 105449817522143713873289191736816315467970330682548964008145209069279544391796791572803429807023304312766876266272581399518725843020156573640514089230038703743453303966007461476603345363277518865734459405413382165767490114833793451131084417922288730672653166013004415494808153304408837782464780024258853874874741165611254153116498208168945386520576105449817522143713873289191736816315467970330682548964008145209069279544391796791572803429807023304312766876266272581399518725843020156573640514089230038703743453303966007461476603345363277518865734459405413382165767490114833793451131084417922288730672653166013004415494808153304408837782464780024258853874874741165611254153116498208168945386520576 11119664015453407423911470850841677212517873383001198413392354081924039546919536443947321421023932952216491468291930561330303057294209197944850772073628172221043557056141564012864876786674023215335376344033744689023793384006507498949746707688469512759652411649584846200937658342898519800643879715710370787512591369109884297533272035603720342242208770829528669744781312794841475567697949443486709168710255509788605343217949407782415509895547510047999063098748721979803123981986534021417715596485373820927744302999115979021508077102746239667504721138780357605056093206892549931859220277447065030190197112782636292403183780149267675435150073268035421951415537393177753260422528426274030236355856193508300065293129340353855270398610364261333353269333825270842514964914317258056955499177826831948297141476723092568167922257054801185415241596640654526757338825475327063533350977559575132457265363436148528613976045115033633080341221591314771064651842115268330323830267764747323506655310898214896923712790468537012248176867952457036441249311684985760748590200638837966464971899773884343524407739943166313428725080441495138855568709527615252705462182094630115812138118680463375871945603021825894941226564583302215885003695893482347349172891621074050386092290115678045620124536916861320784031688091164677807959337122710781904832581878395548785508604001209840512210439533491827461882202822795671371776
	test_bignum mul_negative_pn 6 _ -2 -12
	test_bignum mul_negative_np -3 _ 7 -21
	test_bignum mul_negative_nn -5 _ -3 15
	echo
	test_bignum div_simple 7 / 2 3
	test_bignum div_zero 7 / 0 ""
	test_bignum div_one 7 / 1 7
	test_bignum div_big_small 112233445566778899 / 9 12470382840753211
	test_bignum div_big_big 112233445566778899 / 112233445523 1000000
	test_bignum div_huge_huge 2538687043280516217968757858774735713134894094727501826178087490310416 / 25386870432805162179687578587747357546543 99999999999999999999999999999
	test_bignum div_negative_pn -7 / 3 -2
	test_bignum div_negative_np 13 / -3 -4
	test_bignum div_negative_nn -15 / -4 3
	echo
	test_bignum mod_simple 7 % 2 1
	test_bignum mod_zero 7 % 0 ""
	test_bignum mod_one 7 % 1 0
	test_bignum mod_big_small 112233445566778899 % 15 9
	test_bignum mod_big_big 112233445566778899 % 112233445523 43778899
	test_bignum mod_huge_huge 2538687043280516217968757858774735713134894094727501826178087490310416 % 25386870432805162179687578587747357546543 25386828913399256907189404765834847856959
	test_bignum mod_negative_pn1 12 % -4 -0
	test_bignum mod_negative_pn2 13 % -4 -3
	test_bignum mod_negative_pn3 14 % -4 -2
	test_bignum mod_negative_pn4 15 % -4 -1
	test_bignum mod_negative_pn5 16 % -4 -0
	test_bignum mod_negative_np1 -8 % 3 1
	test_bignum mod_negative_np2 -9 % 3 0
	test_bignum mod_negative_np3 -10 % 3 2
	test_bignum mod_negative_np4 -11 % 3 1
	test_bignum mod_negative_nn1 -14 % -3 -2
	test_bignum mod_negative_nn2 -15 % -3 -0
	test_bignum mod_negative_nn3 -16 % -3 -1
	test_bignum mod_negative_nn4 -17 % -3 -2
	echo
	test_bignum exp_simple 7 ^ 2 49
	test_bignum exp_zero 7 ^ 0 1
	test_bignum exp_one 7 ^ 1 7
	test_bignum exp_big_small 112233445566778899 ^ 15 5647216521307535910774570193454558148429904499572797187291697998843222979973212935737405421983603075035348202218480219580465507643671377868824511584887435285792703089192748536777340314991591476660891321860983157324859273472811684181253631127619682449633499
	test_bignum exp_huge_small 2538687043280516217968757858774735713134894094727501826178087490310416 ^ 5 105449817522143713873289191736816315467970330682548964008145209069279544391796791572803429807023304312766876266272581399518725843020156573640514089230038703743453303966007461476603345363277518865734459405413382165767490114833793451131084417922288730672653166013004415494808153304408837782464780024258853874874741165611254153116498208168945386520576
	test_bignum exp_negative_even -14 ^ 6 7529536
	test_bignum exp_negative_odd -13 ^ 5 -371293
	echo
	test_bignum lshift_simple 7 "<<" 2 28
	test_bignum lshift_zero 7 "<<" 0 7
	test_bignum lshift_one 7 "<<" 1 14
	test_bignum lshift_big_small 112233445566778899 "<<" 13 919416386083052740608
	test_bignum lshift_huge_small 2538687043280516217968757858774735713134894094727501826178087490310416 "<<" 13 20796924258553988857600064379082634962001052424007694960050892720622927872
	test_bignum lshift_negative -23 "<<" 4 -368
	echo
	test_bignum rshift_simple 7 ">>" 2 1
	test_bignum rshift_zero 7 ">>" 0 7
	test_bignum rshift_one 7 ">>" 1 3
	test_bignum rshift_big_small 112233445566778899 ">>" 13 13700371773288
	test_bignum rshift_huge_small 2538687043280516217968757858774735713134894094727501826178087490310416 ">>" 13 309898320712953639888764387057462855607286876797790750265879820594
	test_bignum rshift_negative -43 ">>" 3 -5
	echo
	test_bignum lshift1_small 7 "<<1" osef 14
	test_bignum lshift1_big 112233445566778899 "<<1" osef 224466891133557798
	test_bignum lshift1_huge 2538687043280516217968757858774735713134894094727501826178087490310416 "<<1" osef 5077374086561032435937515717549471426269788189455003652356174980620832
	test_bignum lshift1_negative -33 "<<1" osef -66
	echo
	test_bignum rshift1_small 7 ">>1" osef 3
	test_bignum rshift1_big 112233445566778899 ">>1" osef 56116722783389449
	test_bignum rshift1_huge 2538687043280516217968757858774735713134894094727501826178087490310416 ">>1" osef 1269343521640258108984378929387367856567447047363750913089043745155208
	test_bignum rshift1_negative -54 ">>1" "osef" -27
	echo
	test_bignum is_prime_small_yes 7 is_prime osef 1
	test_bignum is_prime_small_no 8 is_prime osef 0
	test_bignum is_prime_big_yes 32416189853 is_prime osef 1
	test_bignum is_prime_big_no 32416189793 is_prime osef 0
	test_bignum is_prime_huge_yes 170141183460469231731687303715884105727 is_prime osef 1
	test_bignum is_prime_huge_no 170141183460469231731647303715884105727 is_prime osef 0
	test_bignum is_prime_huge_yes 14693679385278593849609206715278070972733319459651094018859396328480215743184089660644531 is_prime osef 1
	test_bignum is_prime_huge_no 14693679385278593849609106715278070972733319459651094018859396328480215743184089660644531 is_prime osef 0
	test_bignum is_prime_huge_yes 43570062353753446053455610056679740005056966111842089407838902783209959981593077811330507328327968191581 is_prime osef 1
	test_bignum is_prime_huge_no 43570062353753446053455610056679740005056966111842089407838902783209959981593077812330507328327968191581 is_prime osef 0
	test_bignum is_prime_huge_yes 173595967255825177671338937551019479316666998993775123240829393474377123659353963383337070902624048512825454641905090447123113341790233547359778733674889927077625565221007272383568497900935065546549429354535026002846488966213929642837090828644809149794625119581137692313935281347548744981442897801650653080519 is_prime osef 1
	test_bignum is_prime_huge_no 173595967255825177671338937551019479316666998993775123240829393474377123659353963373337070902624048512825454641905090447123113341790233547359778733674889927077625565221007272383568497900935065546549429354535026002846488966213929642837090828644809149794625119581137692313935281347548744981442897801650653080519 is_prime osef 0
	echo
	test_bignum_mod mod_add_simple 4 %+ 5 10 9
	test_bignum_mod mod_add_mod 4 %+ 5 4 1
	test_bignum_mod mod_add_neg -4 %+ 7 20 3
	test_bignum_mod mod_add_neg_mod -4 %+ -7 6 1
	echo
	test_bignum_mod mod_sub_simple 15 %- 7 20 8
	test_bignum_mod mod_sub_mod 15 %- 4 6 5
	test_bignum_mod mod_sub_neg 15 %- 20 20 15
	test_bignum_mod mod_sub_neg_mod 20 %- 30 6 2
	echo
	test_bignum_mod mod_mul_simple 8 %* 7 100 56
	test_bignum_mod mod_mul_mod 8 %* 9 20 12
	test_bignum_mod mod_mul_neg -2 %* 4 10 2
	test_bignum_mod mod_mul_neg_mod -4 %* 7 20 12
}

test_genrsa()
{
	$JKSSL_BIN genrsa -out /tmp/jkssl_rsa_key 15 2> /dev/null && print_result_ko genrsa_tooshort || print_result_ok genrsa_tooshort
	$JKSSL_BIN genrsa -out /tmp/jkssl_rsa_key 16 2> /dev/null && print_result_ok genrsa_simple || print_result_ko genrsa_simple
	$OPSSL_BIN rsa -check -in /tmp/jkssl_rsa_key -noout > /dev/null 2> /dev/null && print_result_ok genrsa_check || print_result_ko genrsa_check
}

test_rsautl()
{
	#generate key
	$JKSSL_BIN genrsa -out /tmp/jkssl_rsa_key 512 2> /dev/null
	$OPSSL_BIN genrsa -out /tmp/opssl_rsa_key 512 2> /dev/null

	echo "dummy" > /tmp/jkssl_dummy

	#encrypt with priv
	$JKSSL_BIN rsautl -encrypt -inkey /tmp/jkssl_rsa_key -in /tmp/jkssl_dummy -out /tmp/jkssl_author
	$OPSSL_BIN rsautl -decrypt -inkey /tmp/jkssl_rsa_key -in /tmp/jkssl_author -out /tmp/opssl_author
	df=`diff /tmp/jkssl_dummy /tmp/opssl_author`
	print_result rsautl_encrypt_priv $df ""

	#decrypt
	$OPSSL_BIN rsautl -encrypt -inkey /tmp/jkssl_rsa_key -in /tmp/jkssl_dummy -out /tmp/opssl_author
	$JKSSL_BIN rsautl -decrypt -inkey /tmp/jkssl_rsa_key -in /tmp/opssl_author -out /tmp/jkssl_author_2
	df=`diff /tmp/jkssl_author_2 /tmp/jkssl_dummy`
	print_result rsautl_decrypt $df ""

	#encrypt with public
	$OPSSL_BIN rsa -in /tmp/jkssl_rsa_key -out /tmp/opssl_rsa_key_pub -pubout > /dev/null 2> /dev/null
	$JKSSL_BIN rsautl -encrypt -pubin -inkey /tmp/opssl_rsa_key_pub -in /tmp/jkssl_dummy -out /tmp/jkssl_author_pub
	$OPSSL_BIN rsautl -decrypt -inkey /tmp/jkssl_rsa_key -in /tmp/jkssl_author_pub -out /tmp/opssl_author
	df=`diff /tmp/jkssl_dummy /tmp/opssl_author`
	print_result rsautl_encrypt_pub $df ""

	#decrypt with public
	$JKSSL_BIN rsautl -decrypt -pubin -inkey /tmp/opssl_rsa_key_pub -in /tmp/jkssl_author_pub > /dev/null 2> /dev/null && print_result_ko rsautl_decrypt_public || print_result_ok rsautl_decrypt_public

	#pubin with private key
	$JKSSL_BIN rsautl -encrypt -pubin -inkey /tmp/opssl_rsa_key -in /tmp/jkssl_dummy > /dev/null 2> /dev/null && print_result_ko rsautl_pubin_private || print_result_ok rsautl_pubin_private

	#no pubin with public key
	$JKSSL_BIN rsautl -encrypt -inkey /tmp/opssl_rsa_key_pub -in /tmp/jkssl_dummy > /dev/null 2> /dev/null && print_result_ko rsautl_nopubin_public || print_result_ok rsautl_nopubin_public

	#hexdump
	$OPSSL_BIN rsautl -encrypt -inkey /tmp/jkssl_rsa_key -in /tmp/jkssl_dummy -out /tmp/opssl_author
	$JKSSL_BIN rsautl -in /tmp/opssl_author -decrypt -inkey /tmp/jkssl_rsa_key -out /tmp/jkssl_hexdump_priv -hexdump
	$OPSSL_BIN rsautl -in /tmp/opssl_author -decrypt -inkey /tmp/jkssl_rsa_key -out /tmp/opssl_hexdump_priv -hexdump
	df=`diff /tmp/jkssl_hexdump_priv /tmp/opssl_hexdump_priv`
	print_result rsautl_hexdump $df ""

	#input too long
	$JKSSL_BIN rsautl -encrypt -inkey /tmp/jkssl_rsa_key -in Makefile.am -out /dev/null 2> /dev/null && print_result_ko rsautl_toolong || print_result_ok rsautl_toolong

	#invalid pub
	$JKSSL_BIN rsautl -pubin -inkey /tmp/jkssl_rsa_key -in /tmp/jkssl_dummy > /dev/null 2> /dev/null && print_result_ko rsautl_invalid_pubin || print_result_ok rsautl_invalid_pubin

	#invalid priv
	$JKSSL_BIN rsautl -inkey /tmp/opssl_rsa_key_pub -in /tmp/jkssl_dummy > /dev/null 2> /dev/null && print_result_ko rsautl_invalid_privin || print_result_ok rsautl_invalid_privin

	#invalid decrypt input
	$JKSSL_BIN rsautl -decrypt -inkey /tmp/opssl_rsa_key_pub -in /tmp/jkssl_dummy > /dev/null 2> /dev/null && print_result_ko rsautl_invalid_decrypt_input || print_result_ok rsautl_invalid_decrypt_input

	#test just valid size
	echo -n "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa" | $JKSSL_BIN rsautl -encrypt -inkey /tmp/opssl_rsa_key > /dev/null 2> /dev/null && print_result_ok rsautl_valid_size || print_result_ko rsautl_valid_size

	#test just invalid size
	echo -n "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa" | $JKSSL_BIN rsautl -encrypt -inkey /tmp/opssl_rsa_key > /dev/null 2> /dev/null && print_result_ko rsautl_invalid_size || print_result_ok rsautl_invalid_size

	#sign
	$JKSSL_BIN rsautl -sign -inkey /tmp/jkssl_rsa_key -in /tmp/jkssl_dummy -out /tmp/jkssl_sign
	$OPSSL_BIN rsautl -sign -inkey /tmp/jkssl_rsa_key -in /tmp/jkssl_dummy -out /tmp/opssl_sign
	df=`diff /tmp/jkssl_sign /tmp/opssl_sign`
	print_result rsautl_sign $df ""

	#verify
	$JKSSL_BIN rsautl -verify -inkey /tmp/jkssl_rsa_key -in /tmp/opssl_sign -out /tmp/jkssl_author_verify
	df=`diff /tmp/jkssl_author_verify /tmp/jkssl_dummy`
	print_result rsautl_verify $df ""

	#encrypt
	$JKSSL_BIN rsautl -encrypt -oaep -inkey /tmp/jkssl_rsa_key -in /tmp/jkssl_dummy -out /tmp/jkssl_author
	$OPSSL_BIN rsautl -decrypt -oaep -inkey /tmp/jkssl_rsa_key -in /tmp/jkssl_author -out /tmp/opssl_author
	df=`diff /tmp/jkssl_dummy /tmp/opssl_author`
	print_result rsautl_encrypt_oaep $df ""

	#decrypt
	$OPSSL_BIN rsautl -encrypt -oaep -inkey /tmp/jkssl_rsa_key -in /tmp/jkssl_dummy -out /tmp/opssl_author
	$JKSSL_BIN rsautl -decrypt -oaep -inkey /tmp/jkssl_rsa_key -in /tmp/opssl_author -out /tmp/jkssl_author_2
	df=`diff /tmp/jkssl_author_2 /tmp/jkssl_dummy`
	print_result rsautl_decrypt_oaep $df ""
}

test_rsa()
{
	#generate key
	$JKSSL_BIN genrsa -out /tmp/jkssl_rsa_key 256 > /dev/null 2> /dev/null

	#rsa priv to priv
	$JKSSL_BIN rsa -in /tmp/jkssl_rsa_key -out /tmp/jkssl_rsa_key_priv
	$OPSSL_BIN rsa -in /tmp/jkssl_rsa_key -out /tmp/opssl_rsa_key_priv > /dev/null 2> /dev/null
	df=`diff /tmp/jkssl_rsa_key_priv /tmp/opssl_rsa_key_priv`
	print_result rsa_privtopriv $df ""

	#rsa invalid priv
	$JKSSL_BIN rsa -in /tmp/jkssl_rsa_key_pub > /dev/null 2> /dev/null && print_result_ko rsa_invalid_privin || print_result_ok rsa_invalid_privin

	#rsa check priv
	$JKSSL_BIN rsa -check -in /tmp/jkssl_rsa_key -noout > /dev/null 2> /dev/null && print_result_ok rsa_check_priv_valid || print_result_ko rsa_check_priv_valid
	$JKSSL_BIN rsa -check -in Makefile.am -noout > /dev/null 2> /dev/null && print_result_ko rsa_check_priv_invalid || print_result_ok rsa_check_priv_invalid

	#rsa priv to pub
	$JKSSL_BIN rsa -in /tmp/jkssl_rsa_key -out /tmp/jkssl_rsa_key_pub -pubout
	$OPSSL_BIN rsa -in /tmp/jkssl_rsa_key -out /tmp/opssl_rsa_key_pub -pubout > /dev/null 2> /dev/null
	df=`diff /tmp/jkssl_rsa_key_pub /tmp/opssl_rsa_key_pub`
	print_result rsa_privtopub $df ""

	#rsa pub to pub
	$JKSSL_BIN rsa -pubin -in /tmp/jkssl_rsa_key_pub -out /tmp/jkssl_rsa_key_pub_2 -pubout
	$OPSSL_BIN rsa -pubin -in /tmp/jkssl_rsa_key_pub -out /tmp/opssl_rsa_key_pub_2 -pubout > /dev/null 2> /dev/null
	df=`diff /tmp/jkssl_rsa_key_pub_2 /tmp/opssl_rsa_key_pub_2`
	print_result rsa_pubtopub $df ""

	#rsa pub to priv
	$JKSSL_BIN rsa -pubin -in /tmp/jkssl_rsa_key_pub -out /tmp/jkssl_rsa_key_pub_3
	$OPSSL_BIN rsa -pubin -in /tmp/jkssl_rsa_key_pub -out /tmp/opssl_rsa_key_pub_3 > /dev/null 2> /dev/null
	df=`diff /tmp/jkssl_rsa_key_pub_3 /tmp/opssl_rsa_key_pub_3`
	print_result rsa_pubtopriv $df ""

	#rsa invalid pub
	$JKSSL_BIN rsa -pubin -in /tmp/jkssl_rsa_key > /dev/null 2> /dev/null && print_result_ko rsa_invalid_pubin || print_result_ok rsa_invalid_pubin

	#rsa check pub
	$JKSSL_BIN rsa -pubin -check -in /tmp/jkssl_rsa_key_pub -noout > /dev/null 2> /dev/null && print_result_ok rsa_check_pub_valid || print_result_ko rsa_check_pub_valid
	$JKSSL_BIN rsa -pubin -check -in Makefile.am -noout > /dev/null 2> /dev/null && print_result_ko rsa_check_pub_invalid || print_result_ok rsa_check_pub_invalid

	#rsa priv to rsapub
	$JKSSL_BIN rsa -in /tmp/jkssl_rsa_key -out /tmp/jkssl_rsa_key_pub -RSAPublicKey_out
	$OPSSL_BIN rsa -in /tmp/jkssl_rsa_key -out /tmp/opssl_rsa_key_pub -RSAPublicKey_out > /dev/null 2> /dev/null
	df=`diff /tmp/jkssl_rsa_key_pub /tmp/opssl_rsa_key_pub`
	print_result rsa_privtorsapub $df ""

	#rsa pub to rsapub
	$JKSSL_BIN rsa -RSAPublicKey_in -in /tmp/jkssl_rsa_key_pub -out /tmp/jkssl_rsa_key_pub_2 -RSAPublicKey_out
	$OPSSL_BIN rsa -RSAPublicKey_in -in /tmp/jkssl_rsa_key_pub -out /tmp/opssl_rsa_key_pub_2 -RSAPublicKey_out > /dev/null 2> /dev/null
	df=`diff /tmp/jkssl_rsa_key_pub_2 /tmp/opssl_rsa_key_pub_2`
	print_result rsa_rsapubtorsapub $df ""

	#rsa rsapub to priv
	$JKSSL_BIN rsa -RSAPublicKey_in -in /tmp/jkssl_rsa_key_pub -out /tmp/jkssl_rsa_key_pub_3
	$OPSSL_BIN rsa -RSAPublicKey_in -in /tmp/jkssl_rsa_key_pub -out /tmp/opssl_rsa_key_pub_3 > /dev/null 2> /dev/null
	df=`diff /tmp/jkssl_rsa_key_pub_3 /tmp/opssl_rsa_key_pub_3`
	print_result rsa_rsapubtopriv $df ""

	#rsa invalid rsapub
	$JKSSL_BIN rsa -RSAPublicKey_in -in /tmp/jkssl_rsa_key > /dev/null 2> /dev/null && print_result_ko rsa_invalid_rsapubin || print_result_ok rsa_invalid_rsapubin

	#rsa check rsapub
	$JKSSL_BIN rsa -RSAPublicKey_in -check -in /tmp/jkssl_rsa_key_pub -noout > /dev/null 2> /dev/null && print_result_ok rsa_check_rsapub_valid || print_result_ko rsa_check_rsapub_valid
	$JKSSL_BIN rsa -RSAPublicKey_in -check -in Makefile.am -noout > /dev/null 2> /dev/null && print_result_ko rsa_check_rsapub_invalid || print_result_ok rsa_check_rsapub_invalid
}

test_rsa_signature()
{
	#generate priv
	$JKSSL_BIN genrsa -out /tmp/jkssl_rsa_priv 1024 > /dev/null 2> /dev/null
	#derive pub
	$JKSSL_BIN rsa -in /tmp/jkssl_rsa_priv -out /tmp/jkssl_rsa_pub -pubout
	#sign with private key
	$JKSSL_BIN dgst -sign /tmp/jkssl_rsa_priv -sha1 -out /tmp/jkssl_signature $test_4k
	#verify
	$OPSSL_BIN dgst -verify /tmp/jkssl_rsa_pub -sha1 -signature /tmp/jkssl_signature $test_4k > /dev/null 2> /dev/null && print_result_ok rsa_sign || print_result_ko rsa_sign
	#verify
	$JKSSL_BIN dgst -verify /tmp/jkssl_rsa_pub -sha1 -signature /tmp/jkssl_signature $test_4k && print_result_ok rsa_verify || print_result_ko rsa_verify
	#verify
	$JKSSL_BIN dgst -prverify /tmp/jkssl_rsa_priv -sha1 -signature /tmp/jkssl_signature $test_4k && print_result_ok rsa_prverify || print_result_ko rsa_prverify

	#generate priv
	$JKSSL_BIN genrsa -out /tmp/jkssl_rsa_priv2 1024 > /dev/null 2> /dev/null
	#derive pub
	$JKSSL_BIN rsa -in /tmp/jkssl_rsa_priv2 -out /tmp/jkssl_rsa_pub2 -pubout
	#verify
	$JKSSL_BIN dgst -verify /tmp/jkssl_rsa_pub2 -sha1 -signature /tmp/jkssl_signature $test_4k > /dev/null 2> /dev/null && print_result_ko rsa_verify_invalid || print_result_ok rsa_verify_invalid
	#verify
	$JKSSL_BIN dgst -prverify /tmp/jkssl_rsa_priv2 -sha1 -signature /tmp/jkssl_signature $test_4k > /dev/null 2> /dev/null && print_result_ko rsa_prverify_invalid || print_result_ok rsa_prverify_invalid
}

test_dsaparam()
{
	#generate param
	$JKSSL_BIN dsaparam -out /tmp/jkssl_dsa_param 512 > /dev/null 2> /dev/null

	#dsa param passthrough
	$JKSSL_BIN dsaparam -in /tmp/jkssl_dsa_param -out /tmp/jkssl_dsa_param_2
	$OPSSL_BIN dsaparam -in /tmp/jkssl_dsa_param -out /tmp/opssl_dsa_param > /dev/null 2> /dev/null
	df=`diff /tmp/jkssl_dsa_param_2 /tmp/opssl_dsa_param`
	print_result dsaparam $df ""
}

test_gendsa()
{
	#generate param
	$JKSSL_BIN dsaparam -out /tmp/jkssl_dsa_param 512 > /dev/null 2> /dev/null

	#generate priv
	$JKSSL_BIN gendsa -out /tmp/jkssl_dsa_priv /tmp/jkssl_dsa_param
	$OPSSL_BIN dsa -in /tmp/jkssl_dsa_priv -out /tmp/opssl_dsa_priv > /dev/null 2> /dev/null
	df=`diff /tmp/jkssl_dsa_priv /tmp/opssl_dsa_priv`
	print_result gendsa $df ""
}

test_dsa()
{
	#generate param
	$JKSSL_BIN dsaparam -out /tmp/jkssl_dsa_param 512 > /dev/null 2> /dev/null

	#generate priv
	$JKSSL_BIN gendsa -out /tmp/jkssl_dsa_key /tmp/jkssl_dsa_param

	#dsa priv to priv
	$JKSSL_BIN dsa -in /tmp/jkssl_dsa_key -out /tmp/jkssl_dsa_key_priv
	$OPSSL_BIN dsa -in /tmp/jkssl_dsa_key -out /tmp/opssl_dsa_key_priv > /dev/null 2> /dev/null
	df=`diff /tmp/jkssl_dsa_key_priv /tmp/opssl_dsa_key_priv`
	print_result dsa_privtopriv $df ""

	#dsa priv to pub
	$JKSSL_BIN dsa -in /tmp/jkssl_dsa_key -out /tmp/jkssl_dsa_key_pub -pubout
	$OPSSL_BIN dsa -in /tmp/jkssl_dsa_key -out /tmp/opssl_dsa_key_pub -pubout > /dev/null 2> /dev/null
	df=`diff /tmp/jkssl_dsa_key_pub /tmp/opssl_dsa_key_pub`
	print_result dsa_privtopub $df ""

	#dsa pub to pub
	$JKSSL_BIN dsa -pubin -in /tmp/jkssl_dsa_key_pub -out /tmp/jkssl_dsa_key_pub_2 -pubout
	$OPSSL_BIN dsa -pubin -in /tmp/jkssl_dsa_key_pub -out /tmp/opssl_dsa_key_pub_2 -pubout > /dev/null 2> /dev/null
	df=`diff /tmp/jkssl_dsa_key_pub_2 /tmp/opssl_dsa_key_pub_2`
	print_result dsa_pubtopub $df ""

	#dsa pub to priv
	$JKSSL_BIN dsa -pubin -in /tmp/jkssl_dsa_key_pub -out /tmp/jkssl_dsa_key_pub_3
	$OPSSL_BIN dsa -pubin -in /tmp/jkssl_dsa_key_pub -out /tmp/opssl_dsa_key_pub_3 > /dev/null 2> /dev/null
	df=`diff /tmp/jkssl_dsa_key_pub_3 /tmp/opssl_dsa_key_pub_3`
	print_result dsa_pubtopriv $df ""

}

test_dsa_signature()
{
	#generate param
	$JKSSL_BIN dsaparam -out /tmp/jkssl_dsa_param 512 > /dev/null 2> /dev/null
	#generate priv
	$JKSSL_BIN gendsa -out /tmp/jkssl_dsa_priv /tmp/jkssl_dsa_param
	#derive pub
	$JKSSL_BIN dsa -in /tmp/jkssl_dsa_priv -out /tmp/jkssl_dsa_pub -pubout
	#sign with private key
	$JKSSL_BIN dgst -sign /tmp/jkssl_dsa_priv -sha256 -out /tmp/jkssl_signature $test_4k
	#verify
	$OPSSL_BIN dgst -verify /tmp/jkssl_dsa_pub -sha256 -signature /tmp/jkssl_signature $test_4k > /dev/null 2> /dev/null && print_result_ok dsa_sign || print_result_ko dsa_sign
	#verify
	$JKSSL_BIN dgst -verify /tmp/jkssl_dsa_pub -sha256 -signature /tmp/jkssl_signature $test_4k && print_result_ok dsa_verify || print_result_ko dsa_verify
	#verify
	$JKSSL_BIN dgst -prverify /tmp/jkssl_dsa_priv -sha256 -signature /tmp/jkssl_signature $test_4k && print_result_ok dsa_prverify || print_result_ko dsa_prverify

	#generate priv
	$JKSSL_BIN gendsa -out /tmp/jkssl_dsa_priv2 /tmp/jkssl_dsa_param
	#derive pub
	$JKSSL_BIN dsa -in /tmp/jkssl_dsa_priv2 -out /tmp/jkssl_dsa_pub2 -pubout
	#verify
	$JKSSL_BIN dgst -verify /tmp/jkssl_dsa_pub2 -sha256 -signature /tmp/jkssl_signature $test_4k 2> /dev/null && print_result_ko dsa_verify_invalid || print_result_ok dsa_verify_invalid
	#verify
	$JKSSL_BIN dgst -prverify /tmp/jkssl_dsa_priv2 -sha256 -signature /tmp/jkssl_signature $test_4k 2> /dev/null && print_result_ko dsa_prverify_invalid || print_result_ok dsa_prverify_invalid
}

test_dhparam()
{
	#generate param
	$JKSSL_BIN dhparam -out /tmp/jkssl_dh_param 512 > /dev/null 2> /dev/null

	#param to param
	$JKSSL_BIN dhparam -in /tmp/jkssl_dh_param -out /tmp/jkssl_dh_param_2
	$OPSSL_BIN dhparam -in /tmp/jkssl_dh_param -out /tmp/opssl_dh_param > /dev/null 2> /dev/null
	df=`diff /tmp/jkssl_dh_param_2 /tmp/opssl_dh_param`
	print_result dhparam $df ""
}

has_printed_ln=0

nl_if_not_first()
{
	if [ "$has_printed_ln" = 0 ]
	then
		has_printed_ln=1
	else
		printf "\n"
	fi
}

ops=${@:-"hash base64 des aes camellia aria chacha20 rc4 rc2 seed bf cast5 sm4 bignum genrsa rsautl rsa rsa_signature dsaparam gendsa dsa dsa_sign dsa_verify dhparam"}

for var in $ops
do
	case $var in
		"hash")
			nl_if_not_first
			test_hash_all
			;;
		"base64")
			nl_if_not_first
			test_base64_all
			;;
		"des")
			nl_if_not_first
			test_des
			;;
		"des1")
			nl_if_not_first
			test_des1
			;;
		"des2")
			nl_if_not_first
			test_des2
			;;
		"des3")
			nl_if_not_first
			test_des3
			;;
		"aes")
			nl_if_not_first
			test_aes
			;;
		"aes-128")
			nl_if_not_first
			test_aes128
			;;
		"aes-192")
			nl_if_not_first
			test_aes192
			;;
		"aes-256")
			nl_if_not_first
			test_aes256
			;;
		"camellia")
			nl_if_not_first
			test_camellia
			;;
		"camellia-128")
			nl_if_not_first
			test_camellia128
			;;
		"camellia-192")
			nl_if_not_first
			test_camellia192
			;;
		"camellia-256")
			nl_if_not_first
			test_camellia256
			;;
		"aria")
			nl_if_not_first
			test_aria
			;;
		"aria-128")
			nl_if_not_first
			test_aria128
			;;
		"aria-192")
			nl_if_not_first
			test_aria192
			;;
		"aria-256")
			nl_if_not_first
			test_aria256
			;;
		"chacha20")
			nl_if_not_first
			test_chacha20
			;;
		"rc4")
			nl_if_not_first
			test_rc4
			;;
		"rc2")
			nl_if_not_first
			test_rc2
			;;
		"seed")
			nl_if_not_first
			test_seed
			;;
		"bf")
			nl_if_not_first
			test_bf
			;;
		"cast5")
			nl_if_not_first
			test_cast5
			;;
		"sm4")
			nl_if_not_first
			test_sm4
			;;
		"enc")
			nl_if_not_first
			test_enc
			;;
		"bignum")
			nl_if_not_first
			test_bignum_all
			;;
		"genrsa")
			nl_if_not_first
			test_genrsa
			;;
		"rsautl")
			nl_if_not_first
			test_rsautl
			;;
		"rsa")
			nl_if_not_first
			test_rsa
			;;
		"rsa_signature")
			nl_if_not_first
			test_rsa_signature
			;;
		"dsaparam")
			nl_if_not_first
			test_dsaparam
			;;
		"gendsa")
			nl_if_not_first
			test_gendsa
			;;
		"dsa")
			nl_if_not_first
			test_dsa
			;;
		"dsa_signature")
			nl_if_not_first
			test_dsa_signature
			;;
		"dhparam")
			nl_if_not_first
			test_dhparam
			;;
	esac
done

rm $test_4k
rm $test_1M
rm $test_16M
