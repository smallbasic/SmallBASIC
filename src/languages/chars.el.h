#define CHS_UPPER "ABCDEFGHIJKLMNOPQRSTUVWXYZÁ¶ÂÃÄÅ¸ÆÇ¹ÈÉºÚÊËÌÍÎÏ¼ÐÑÓÓÔÕ¾ÛÖ×ØÙ¿"
#define CHS_LOWER "abcdefghijklmnopqrstuvwxyzáÜâãäåÝæçÞèéßúêëìíîïüðñóòôõýûö÷øùþ"

#define	is_digit(c)	((c) >= 48 && (c) <= 57)												/**< true if the character is a digit @ingroup str */
#define	is_upper(c)	(strchr(CHS_UPPER, (c)) != NULL)
#define	is_lower(c)	(strchr(CHS_LOWER, (c)) != NULL)
#define	to_upper(c)	(char_table_replace(CHS_LOWER, (c), CHS_UPPER))
#define	to_lower(c)	(char_table_replace(CHS_UPPER, (c), CHS_LOWER))
#define	is_hexdigit(c)	(is_digit((c)) || (to_upper((c)) >= 65 && to_upper((c)) <= 70))		/**< true if the character is a hexadecimal digit @ingroup str */
#define	is_octdigit(c)	((c) >= '0' && (c) <= '7')											/**< true if the character is an octadecimal digit @ingroup str */
#define	to_hexdigit(c)	( ( ((c) & 0xF) > 9)? ((c)-10)+'A' : (c)+'0' )								/**< returns the hex-digit of the 4-bit number c @ingroup str */

