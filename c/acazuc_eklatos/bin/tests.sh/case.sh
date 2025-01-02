#!/bin/sh
case t in 't' | 'u') echo OK;; 'c')echo KO;esac
case u in ('t' | 'u') echo OK;; 'c')echo KO;esac
case c in 't' | 'u') echo KO;; 'c')echo OK;esac
case a in
	b)
		echo KO
		;;
	a)
		echo OK
		;;
	c)
		echo KO
		;;
	a)
		echo KO
		;;
esac
