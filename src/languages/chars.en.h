#define	is_digit(c)	((c) >= 48 && (c) <= 57)												/**< true if the character is a digit @ingroup str */
#define	is_upper(c)	((c) >= 65 && (c) <= 90)												/**< true if the character is upper-case @ingroup str */
#define	is_lower(c)	((c) >= 97 && (c) <= 122)												/**< true if the character is lower-case @ingroup str */
#define	to_upper(c)	(((c) >= 97 && (c) <= 122) ? (c) - 32 : (c))							/**< returns the upper-case character @ingroup str */
#define	to_lower(c)	(((c) >= 65 && (c) <= 90) ? (c) + 32 : (c))								/**< returns the lower-case character @ingroup str */
#define	is_hexdigit(c)	(is_digit((c)) || (to_upper((c)) >= 65 && to_upper((c)) <= 70))		/**< true if the character is a hexadecimal digit @ingroup str */
#define	is_octdigit(c)	((c) >= '0' && (c) <= '7')											/**< true if the character is an octadecimal digit @ingroup str */
#define	to_hexdigit(c)	( ( ((c) & 0xF) > 9)? ((c)-10)+'A' : (c)+'0' )								/**< returns the hex-digit of the 4-bit number c @ingroup str */

