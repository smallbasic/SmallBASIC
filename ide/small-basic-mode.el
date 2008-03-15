;; small-basic-mode.el --- A mode for editing Small Basic programs.

;; Original Visual Basic header...
;; Copyright (C) 1996 Fred White <fwhite@alum.mit.edu>

;; Author: Fred White <fwhite@alum.mit.edu>
;; Version: 1.3 (May 1, 1996)
;; Keywords: languages basic

;; (Old) LCD Archive Entry:
;; basic-mode|Fred White|fwhite@alum.mit.edu|
;; A mode for editing Small Basic programs.|
;; 18-Apr-96|1.0|~/modes/basic-mode.el.Z|

;; This file is NOT part of GNU Emacs but the same permissions apply.
;;
;; GNU Emacs  is free software;  you can redistribute it and/or modify
;; it under the terms of  the GNU General  Public License as published
;; by  the Free Software  Foundation;  either version  2, or (at  your
;; option) any later version.
;;
;; GNU  Emacs is distributed  in the hope that  it will be useful, but
;; WITHOUT    ANY  WARRANTY;  without even the     implied warranty of
;; MERCHANTABILITY or FITNESS FOR A  PARTICULAR PURPOSE.  See the  GNU
;; General Public License for more details.
;;
;; You should have received  a copy of  the GNU General Public License
;; along with GNU Emacs; see  the file COPYING.  If  not, write to the
;; Free Software Foundation, 675  Mass Ave, Cambridge, MA 02139,  USA.
;; This  program  is free  software;  you  can  redistribute it and/or
;; modify it  under  the terms of the  GNU  General Public License  as
;; published by the Free Software  Foundation; either version 2 of the
;; License, or (at your option) any later version.


;; Purpose of this package:
;;  This is a mode for editing programs written in The World's Most
;;  Successful Programming Language.  It features automatic
;;  indentation, font locking, keyword capitalization, and some minor
;;  convenience functions.

;; Installation instructions
;;  Put basic-mode.el somewhere in your path, compile it, and add the
;;  following to your init file:

;;  (autoload 'small-basic-mode "small-basic-mode" "Small Basic mode." t)
;;  (setq auto-mode-alist (append '(("\\.\\(frm\\|bas\\|cls\\)$" . 
;;                                  small-basic-mode)) auto-mode-alist))

;(setq load-path (append "c:\sb" load-path))
;(autoload 'small-basic-mode "small-basic-mode" "Small Basic mode." t)
;(setq auto-mode-alist (append '(("\\.bas\\'" . small-basic-mode))
;  auto-mode-alist))

;; Of course, under Windows 3.1, you'll have to name this file
;; something shorter than small-basic-mode.el

;; Revisions:
;; 1.0 18-Apr-96  Initial version
;; 1.1 Accomodate emacs 19.29+ font-lock-defaults
;;     Simon Marshall <Simon.Marshall@esrin.esa.it>
;  1.2 Rename to small-basic-mode
;; 1.3 Fix some indentation bugs.
;; changes by G.U. Lauri
;; 1.4 Added automatic header comment construction.
;;     vorking out origina code coming from NTEmacs Mailing List

;; Known bugs:
;;  Doesn't know about ":" separated stmts
;;  Doesn't know about single-line IF stmts


;; todo:
;;  fwd/back-compound-statement
;;  completion over OCX methods and properties.
;;  ensure Then at the end of IF statements.
;;  IDE integration
;;  etc.


(provide 'small-basic-mode)

(defvar small-basic-xemacs-p (string-match "XEmacs\\|Lucid" (emacs-version)))
(defvar small-basic-winemacs-p (string-match "Win-Emacs" (emacs-version)))
(defvar small-basic-win32-p (eq window-system 'win32))

;; Variables you may want to customize.
(defvar small-basic-mode-indent 4 "*Default indentation per nesting level")
(defvar small-basic-fontify-p t "*Whether to fontify Basic buffers.")
(defvar small-basic-capitalize-keywords-p t
  "*Whether to capitalize BASIC keywords.")
(defvar small-basic-wild-files "*.frm *.bas *.cls"
  "*Wildcard pattern for BASIC source files")
(defvar small-basic-ide-pathname nil
  "*The full pathname of your Small Basic exe file, if any.")


(defvar small-basic-keywords-to-highlight
  '("dim" "if" "then" "else" "elseif" "elif" "endif" "fi" )
  "*A list of keywords to highlight in Basic mode, or T, meaning all keywords")

(defvar small-basic-defn-templates
  (list "Public Sub ()\nEnd Sub\n\n"
	"Public Function () As Variant\nEnd Function\n\n"
	"Public Property Get ()\nEnd Property\n\n")
  "*List of function templates though which small-basic-new-sub cycles.")



(defvar small-basic-mode-syntax-table nil)
(if small-basic-mode-syntax-table
    ()
  (setq small-basic-mode-syntax-table (make-syntax-table))
  (modify-syntax-entry ?\' "\<" small-basic-mode-syntax-table) ; Comment starter
  (modify-syntax-entry ?\n ">" small-basic-mode-syntax-table)
  (modify-syntax-entry ?\\ "w" small-basic-mode-syntax-table)
  (modify-syntax-entry ?_ "w" small-basic-mode-syntax-table))


(defvar small-basic-mode-map nil)
(if small-basic-mode-map
    ()
  (setq small-basic-mode-map (make-sparse-keymap))
  (define-key small-basic-mode-map "\t" 'small-basic-indent-line)
  (define-key small-basic-mode-map "\r" 'small-basic-newline-and-indent)
  (define-key small-basic-mode-map "\M-\C-a" 'small-basic-beginning-of-defun)
  (define-key small-basic-mode-map "\M-\C-e" 'small-basic-end-of-defun)
  (define-key small-basic-mode-map "\M-\C-h" 'small-basic-mark-defun)
  (define-key small-basic-mode-map "\M-\C-\\" 'small-basic-indent-region)
  (define-key small-basic-mode-map "\M-q" 'small-basic-fill-or-indent)
  (define-key small-basic-mode-map "\M-\C-q" 'small-basic-comment-function)
  (cond (small-basic-winemacs-p
	 (define-key small-basic-mode-map '(control C) 'small-basic-start-ide))
	(small-basic-win32-p
	 (define-key small-basic-mode-map (read "[?\\S-\\C-c]") 'small-basic-start-ide)))
  (if small-basic-xemacs-p
      (progn
	(define-key small-basic-mode-map "\M-G" 'small-basic-grep)
	(define-key small-basic-mode-map '(meta backspace) 'backward-kill-word)
	(define-key small-basic-mode-map '(control meta /) 'small-basic-new-sub))))


;; These abbrevs are valid only in a code context.
(defvar small-basic-mode-abbrev-table nil)

(defvar small-basic-mode-hook ())


;; Is there a way to case-fold all regexp matches?

(defconst small-basic-defun-start-regexp
  (concat
   "^[ \t]*\\([Pp]ublic \\|[Pp]rivate \\|[Ss]tatic \\)*"
   "\\([Ss]ub\\|[Ff]unction\\|[Pp]roperty +[GgSsLl]et\\|[Tt]ype\\)"
   "[ \t]+\\(\\w+\\)[ \t]*(?"))

(defconst small-basic-defun-end-regexp
  "^[ \t]*[Ee]nd")


;; Includes the compile-time #if variation.
(defconst small-basic-if-regexp "^[ \t]*#?[Ii]f")
(defconst small-basic-else-regexp "^[ \t]*#?[Ee]lse\\([Ii]f\\)?")
(defconst small-basic-endif-regexp "[ \t]*#?[Ff][Ii]")

(defconst small-basic-continuation-regexp "^.*\\_[ \t]*$")
(defconst small-basic-label-regexp "^[ \t]*[a-zA-Z0-9_]+:$")

(defconst small-basic-select-regexp "^[ \t]*[Ss]elect[ \t]+[Cc]ase")
(defconst small-basic-case-regexp "^[ \t]*[Cc]ase")
(defconst small-basic-select-end-regexp "^[ \t]*[Ee]nd[ \t]+[Ss]elect")

(defconst small-basic-for-regexp "^[ \t]*[Ff]or\\b")
(defconst small-basic-next-regexp "^[ \t]*[Nn]ext\\b")

(defconst small-basic-do-regexp "^[ \t]*[Dd]o\\b")
(defconst small-basic-loop-regexp "^[ \t]*[Ll]oop\\b")

(defconst small-basic-while-regexp "^[ \t]*[Ww]hile\\b")
(defconst small-basic-wend-regexp "^[ \t]*[Ww]end\\b")

(defconst small-basic-with-regexp "^[ \t]*[Ww]ith\\b")
(defconst small-basic-end-with-regexp "^[ \t]*[Ee]nd[ \t]+[Ww]ith\\b")

(defconst small-basic-blank-regexp "^[ \t]*$")
(defconst small-basic-comment-regexp "^[ \t]*\\s<.*$")

;; keywords copied from kw.h
(defconst small-basic-all-keywords
'("local"
  "func"
  "proc"
  "byref"
  "declare"
  "import"
  "let"
  "const"
  "end"
  "stop"
  "print"
  "using"
  "input"
  "sinput"
  "inputsep"
  "loopsep"
  "procsep"
  "funcsep"
  "rem"
  "label"
  "goto"
  "if"
  "then"
  "else"
  "elseif"
  "elif"
  "endif"
  "fi"
  "for"
  "to"
  "step"
  "in"
  "next"
  "while"
  "wend"
  "repeat"
  "until"
  "gosub"
  "return"
  "exit"
  "loop"
  "dim"
  "redim"
  "chain"
  "read"
  "restore"			
  "data"
  "color"
  "filled"
  "line"
  "on"
  "off"
  "tron"
  "troff"
  "onjmp"
  "run"
  "exec"
  "erase"
  "use"
  "forsep"
  "outputsep"
  "append"
  "insert"
  "delete"
  "appendsep"
  "open"
  "as"
  "fileprint"
  "lineinput"
  "fileinput"
  "filewrite"
  "fileread"
  "close"
  "scrmode"
  "seek"
  "access"
  "shared"
  "type"
  "sprint"
  "do"
  "option"
  "cls"
  "rte"
  "shell"
  "environ"
  "locate"
  "at"
  "pen"
  "datedmy"
  "beep"
  "sound"
  "pset"
  "rect"
  "circle"
  "randomize"
  "split"
  "wsplit"
  "wjoin"
  "pause"
  "delay"
  "arc"
  "draw"
  "paint"
  "play"
  "sort"
  "search"
  "root"
  "diffeq"
  "chart"
  "window"
  "view"
  "drawpoly"
  "m3ident"
  "m3rotate"
  "m3scale"
  "m3translate"
  "m3apply"
  "segintersect"
  "polyext"
  "deriv"
  "loadln"
  "saveln"
  "kill"
  "rename"
  "copy"
  "chdir"
  "mkdir"
  "rmdir"
  "flock"
  "chmod"
  "plot"
  "logprint"
  "stkdump"
  "swap"
  "button"
  "text"
  "doform"
  "dirwalk"
  "asc"
  "val"
  "chr"
  "str"
  "oct"
  "hex"
  "lcase"
  "ucase"
  "ltrim"
  "rtrim"
  "space"
  "tab"
  "cat"
  "environf"
  "trim"
  "string"
  "squeeze"
  "left"	
  "right"
  "leftof"
  "rightof"
  "leftoflast"
  "rightoflast"
  "mid"
  "replace"
  "runf"
  "inkey"
  "time"
  "date"
  "instr"
  "rinstr"
  "lbound"
  "ubound"
  "len"
  "empty"
  "isarray"
  "isnumber"
  "isstring"
  "atan2"
  "pow"
  "round"
  "cos"
  "sin"
  "tan"
  "cosh"
  "sinh"
  "tanh"
  "acos"
  "asin"
  "atan"
  "acosh"
  "asinh"
  "atanh"
  "sqr"
  "abs"
  "exp"
  "log"
  "log10"
  "fix"
  "int"
  "cdbl"
  "deg"
  "rad"
  "penf"
  "floor"
  "ceil"
  "frac"
  "fre"
  "sgn"
  "cint"
  "eof"
  "seekf"
  "lof"
  "rnd"
  "max"			
  "min"
  "absmax"		
  "absmin"
  "sum"			
  "sumsv"
  "statmean"
  "statmeandev"
  "statspreads"
  "statspreadp"
  "segcos"
  "segsin"
  "seglen"
  "polyarea"
  "ptdistseg"
  "ptsign"
  "ptdistln"
  "point"
  "codearray"
  "gaussjordan"
  "files"
  "inverse"
  "determ"
  "julian"
  "datefmt"
  "wday"
  "iff"
  "format"
  "freefile"
  "ticks"
  "tickspersec"
  "timer" 
  "progline"
  "inputf"
  "textwidth"
  "textheight"
  "exist"
  "isfile"
  "isdir"
  "islink"
  "accessf"
  "xpos"
  "ypos"
  "rgb"
  "rgbf"
  "bin"
  "enclose"
  "disclose"
  "searchf"
  "translatef"
  "chop"
  "tload"
  "tsave"
))

(defun small-basic-word-list-regexp (keys)
  (let ((re "\\b\\(")
	(key nil))
    (while keys
      (setq key (car keys)
	    keys (cdr keys))
      (setq re (concat re key (if keys "\\|" ""))))
    (concat re "\\)\\b")))

(defun small-basic-keywords-to-highlight ()
  (if t
      small-basic-all-keywords
    small-basic-keywords-to-highlight))


(defvar small-basic-font-lock-keywords
  (list
   ;; Names of functions.
   (list small-basic-defun-start-regexp 3 'font-lock-function-name-face)

   ;; Statement labels
   (cons small-basic-label-regexp 'font-lock-reference-face)

   ;; Case values
   ;; String-valued cases get font-lock-string-face regardless.
   (list "^[ \t]*[Cc]ase[ \t]+\\([^'\n]+\\)" 1 'font-lock-keyword-face t)

   ;; Any keywords you like.
   (cons (small-basic-word-list-regexp (small-basic-keywords-to-highlight))
	 'font-lock-keyword-face)))


(put 'small-basic-mode 'font-lock-keywords 'small-basic-font-lock-keywords)

(defun small-basic-mode ()
  "A mode for editing Small Basic programs.
Features automatic  indentation, font locking, keyword capitalization, 
and some minor convenience functions.
Commands:
\\{small-basic-mode-map}"
  (interactive)
  (kill-all-local-variables)
  (use-local-map small-basic-mode-map)
  (setq major-mode 'small-basic-mode)
  (setq mode-name "Small Basic")
  (set-syntax-table small-basic-mode-syntax-table)

  (add-hook 'write-file-hooks 'small-basic-untabify)

  (setq local-abbrev-table small-basic-mode-abbrev-table)
  (if small-basic-capitalize-keywords-p
      (progn
	(make-local-variable 'pre-abbrev-expand-hook)
	(add-hook 'pre-abbrev-expand-hook 'small-basic-pre-abbrev-expand-hook)
	(abbrev-mode 1)))

  (make-local-variable 'comment-start)
  (setq comment-start "' ")
  (make-local-variable 'comment-start-skip)
  (setq comment-start-skip "'+ *")
  (make-local-variable 'comment-column)
  (setq comment-column 40)
  (make-local-variable 'comment-end)
  (setq comment-end "")

  (make-local-variable 'indent-line-function)
  (setq indent-line-function 'small-basic-indent-line)

  (if small-basic-fontify-p
      (small-basic-enable-font-lock))

  (run-hooks 'small-basic-mode-hook))


(defun small-basic-enable-font-lock ()
  ;; Emacs 19.29 requires a window-system else font-lock-mode errs out.
  (cond ((or small-basic-xemacs-p window-system)

	 ;; In win-emacs this sets font-lock-keywords back to nil!
	 (if small-basic-winemacs-p
	     (font-lock-mode 1))

	 ;; Accomodate emacs 19.29+
	 ;; From: Simon Marshall <Simon.Marshall@esrin.esa.it>
	 (cond ((boundp 'font-lock-defaults)
		(make-local-variable 'font-lock-defaults)
		(setq font-lock-defaults '(small-basic-font-lock-keywords)))
	       (t
		(make-local-variable 'font-lock-keywords)
		(setq font-lock-keywords small-basic-font-lock-keywords)))

	 (if small-basic-winemacs-p
	     (font-lock-fontify-buffer)
	   (font-lock-mode 1)))))


(defun small-basic-construct-keyword-abbrev-table ()
  (if small-basic-mode-abbrev-table
      nil
    (let ((words small-basic-all-keywords)
	  (word nil)
	  (list nil))
      (while words
	(setq word (car words)
	      words (cdr words))
	(setq list (cons (list (downcase word) word) list)))

      (define-abbrev-table 'small-basic-mode-abbrev-table list))))

;; Would like to do this at compile-time.
(small-basic-construct-keyword-abbrev-table)


(defun small-basic-in-code-context-p ()
  (if (fboundp 'buffer-syntactic-context) ; XEmacs function.
      (null (buffer-syntactic-context))
    ;; Attempt to simulate buffer-syntactic-context
    ;; I don't know how reliable this is.
    (let* ((beg (save-excursion
		  (beginning-of-line)
		  (point)))
	   (list
	    (parse-partial-sexp beg (point))))
      (and (null (nth 3 list))		; inside string.
	   (null (nth 4 list))))))	; inside cocmment

(defun small-basic-pre-abbrev-expand-hook ()
  ;; Allow our abbrevs only in a code context.
  (setq local-abbrev-table
	(if (small-basic-in-code-context-p)
	    small-basic-mode-abbrev-table)))
	 
	

(defun small-basic-newline-and-indent (&optional count)
  "Insert a newline, updating indentation."
  (interactive)
  (expand-abbrev)
  (save-excursion
    (small-basic-indent-line))
  (call-interactively 'newline-and-indent))
  
(defun small-basic-beginning-of-defun ()
  (interactive)
  (re-search-backward small-basic-defun-start-regexp))

(defun small-basic-end-of-defun ()
  (interactive)
  (re-search-forward small-basic-defun-end-regexp))

(defun small-basic-mark-defun ()
  (interactive)
  (beginning-of-line)
  (small-basic-end-of-defun)
  (set-mark (point))
  (small-basic-beginning-of-defun)
  (if small-basic-xemacs-p
      (zmacs-activate-region)))

(defun small-basic-indent-defun ()
  (interactive)
  (save-excursion
    (small-basic-mark-defun)
    (call-interactively 'small-basic-indent-region)))


(defun small-basic-fill-long-comment ()
  "Fills block of comment lines around point."
  ;; Derived from code in ilisp-ext.el.
  (interactive)
  (save-excursion
    (beginning-of-line)
    (let ((comment-re "^[ \t]*\\s<+[ \t]*"))
      (if (looking-at comment-re)
	  (let ((fill-prefix
		 (buffer-substring
		  (progn (beginning-of-line) (point))
		  (match-end 0))))

	    (while (and (not (bobp))
			(looking-at small-basic-comment-regexp))
	      (forward-line -1))
	    (if (not (bobp)) (forward-line 1))

	    (let ((start (point)))

	      ;; Make all the line prefixes the same.
	      (while (and (not (eobp))
			  (looking-at comment-re))
		(replace-match fill-prefix)
		(forward-line 1))

	      (if (not (eobp))
		  (beginning-of-line))

	      ;; Fill using fill-prefix
	      (fill-region-as-paragraph start (point))))))))


(defun small-basic-fill-or-indent ()
  "Fill long comment around point, if any, else indent current definition."
  (interactive)
  (cond ((save-excursion
	   (beginning-of-line)
	   (looking-at small-basic-comment-regexp))
	 (small-basic-fill-long-comment))
	(t
	 (small-basic-indent-defun))))


(defun small-basic-new-sub ()
  "Insert template for a new subroutine. Repeat to cycle through alternatives."
  (interactive)
  (beginning-of-line)
  (let ((templates (cons small-basic-blank-regexp
			 small-basic-defn-templates))
	(tem nil)
	(bound (point)))
    (while templates
      (setq tem (car templates)
	    templates (cdr templates))
      (cond ((looking-at tem)
	     (replace-match (or (car templates)
				""))
	     (setq templates nil))))

    (search-backward "()" bound t)))


(defun small-basic-untabify ()
  "Do not allow any tabs into the file"
  (if (eq major-mode 'small-basic-mode)
      (untabify (point-min) (point-max)))
  nil)

(defun small-basic-default-tag ()
  (if (and (not (bobp))
	   (save-excursion
	     (backward-char 1)
	     (looking-at "\\w")))
      (backward-word 1))
  (let ((s (point))
	(e (save-excursion
	     (forward-word 1)
	     (point))))
    (buffer-substring s e)))

(defun small-basic-grep (tag)
  "Search BASIC source files in current directory for tag."
  (interactive
   (list (let* ((def (small-basic-default-tag))
		(tag (read-string
		      (format "Grep for [%s]: " def))))
	   (if (string= tag "") def tag))))
  (grep (format "grep -n %s %s" tag small-basic-wild-files)))


;;; IDE Connection.

(defun small-basic-buffer-project-file ()
  "Return a guess as to the project file associated with the current buffer."
  (car (directory-files (file-name-directory (buffer-file-name)) t "\\.vbp")))

(defun small-basic-start-ide ()
  "Start Small Basic (or your favorite IDE, (after Emacs, of course))
on the first project file in the current directory.
Note: it's not a good idea to leave Small Basic running while you
are editing in emacs, since Small Basic has no provision for reloading
changed files."
  (interactive)
  (let (file)
    (cond ((null small-basic-ide-pathname)
	   (error "No pathname set for Small Basic. See small-basic-ide-pathname"))
	  ((null (setq file (small-basic-buffer-project-file)))
	   (error "No project file found."))
	  ((fboundp 'win-exec)
	   (iconify-emacs)
	   (win-exec small-basic-ide-pathname 'win-show-normal file))
	  ((fboundp 'start-process)
	   (iconify-frame (selected-frame))
	   (start-process "*SmallBasic*" nil small-basic-ide-pathname file))
	  (t
	   (error "No way to spawn process!")))))



;;; Indentation-related stuff.

(defun small-basic-indent-region (start end)
  "Perform small-basic-indent-line on each line in region."
  (interactive "r")
  (save-excursion
    (goto-char start)
    (beginning-of-line)
    (while (and (not (eobp))
		(< (point) end))
      (if (not (looking-at small-basic-blank-regexp))
	  (small-basic-indent-line))
      (forward-line 1)))

  (cond ((fboundp 'zmacs-deactivate-region)
	 (zmacs-deactivate-region))
	((fboundp 'deactivate-mark)
	 (deactivate-mark))))



(defun small-basic-previous-line-of-code ()
  (if (not (bobp))
      (forward-line -1))	; previous-line depends on goal column
  (while (and (not (bobp))
	      (or (looking-at small-basic-blank-regexp)
		  (looking-at small-basic-comment-regexp)))
    (forward-line -1)))


(defun small-basic-find-original-statement ()
  ;; If the current line is a continuation from the previous, move
  ;; back to the original stmt.
  (let ((here (point)))
    (small-basic-previous-line-of-code)
    (while (and (not (bobp))
		(looking-at small-basic-continuation-regexp))
      (setq here (point))
      (small-basic-previous-line-of-code))
    (goto-char here)))

(defun small-basic-find-matching-stmt (open-regexp close-regexp)
  ;; Searching backwards
  (let ((level 0))
    (while (and (>= level 0) (not (bobp)))
      (small-basic-previous-line-of-code)
      (small-basic-find-original-statement)
      (cond ((looking-at close-regexp)
	     (setq level (+ level 1)))
	    ((looking-at open-regexp)
	     (setq level (- level 1)))))))

(defun small-basic-find-matching-if ()
  (small-basic-find-matching-stmt small-basic-if-regexp small-basic-endif-regexp))

(defun small-basic-find-matching-select ()
  (small-basic-find-matching-stmt small-basic-select-regexp small-basic-select-end-regexp))

(defun small-basic-find-matching-for ()
  (small-basic-find-matching-stmt small-basic-for-regexp small-basic-next-regexp))

(defun small-basic-find-matching-do ()
  (small-basic-find-matching-stmt small-basic-do-regexp small-basic-loop-regexp))

(defun small-basic-find-matching-while ()
  (small-basic-find-matching-stmt small-basic-while-regexp small-basic-wend-regexp))

(defun small-basic-find-matching-with ()
  (small-basic-find-matching-stmt small-basic-with-regexp small-basic-end-with-regexp))


(defun small-basic-calculate-indent ()
  (let ((original-point (point)))
    (save-excursion
      (beginning-of-line)
      ;; Some cases depend only on where we are now.
      (cond ((or (looking-at small-basic-defun-start-regexp)
		 (looking-at small-basic-label-regexp)
		 (looking-at small-basic-defun-end-regexp))
	     0)

	    ;; The outdenting stmts, which simply match their original.
	    ((or (looking-at small-basic-else-regexp)
		 (looking-at small-basic-endif-regexp))
	     (small-basic-find-matching-if)
	     (current-indentation))

	    ;; All the other matching pairs act alike.
	    ((looking-at small-basic-next-regexp) ; for/next
	     (small-basic-find-matching-for)
	     (current-indentation))

	    ((looking-at small-basic-loop-regexp) ; do/loop
	     (small-basic-find-matching-do)
	     (current-indentation))

	    ((looking-at small-basic-wend-regexp) ; while/wend
	     (small-basic-find-matching-while)
	     (current-indentation))

	    ((looking-at small-basic-end-with-regexp) ; with/end with
	     (small-basic-find-matching-with)
	     (current-indentation))

	    ((looking-at small-basic-select-end-regexp) ; select case/end select
	     (small-basic-find-matching-select)
	     (current-indentation))

	    ;; A case of a select is somewhat special.
	    ((looking-at small-basic-case-regexp)
	     (small-basic-find-matching-select)
	     (+ (current-indentation) small-basic-mode-indent))

	    (t
	     ;; Other cases which depend on the previous line.
	     (small-basic-previous-line-of-code)

	     ;; Skip over label lines, which always have 0 indent.
	     (while (looking-at small-basic-label-regexp)
	       (small-basic-previous-line-of-code))

	     (cond 
	      ((looking-at small-basic-continuation-regexp)
	       (small-basic-find-original-statement)
	       ;; Indent continuation line under matching open paren,
	       ;; or else one word in.
	       (let* ((orig-stmt (point))
		      (matching-open-paren
		       (condition-case ()
			   (save-excursion
			     (goto-char original-point)
			     (beginning-of-line)
			     (backward-up-list 1)
			     ;; Only if point is now w/in cont. block.
			     (if (<= orig-stmt (point))
				 (current-column)))
			 (error nil))))
		 (cond (matching-open-paren
			(1+ matching-open-paren))
		       (t
			;; Else, after first word on original line.
			(back-to-indentation)
			(forward-word 1)
			(while (looking-at "[ \t]")
			  (forward-char 1))
			(current-column)))))
	      (t
	       (small-basic-find-original-statement)

	       (let ((indent (current-indentation)))
		 ;; All the various +indent regexps.
		 (cond ((looking-at small-basic-defun-start-regexp)
			(+ indent small-basic-mode-indent))

		       ((or (looking-at small-basic-if-regexp)
			    (looking-at small-basic-else-regexp))
			(+ indent small-basic-mode-indent))

		       ((or (looking-at small-basic-select-regexp)
			    (looking-at small-basic-case-regexp))
			(+ indent small-basic-mode-indent))
			
		       ((or (looking-at small-basic-do-regexp)
			    (looking-at small-basic-for-regexp)
			    (looking-at small-basic-while-regexp)
			    (looking-at small-basic-with-regexp))
			(+ indent small-basic-mode-indent))

		       (t
			;; By default, just copy indent from prev line.
			indent))))))))))

(defun small-basic-indent-to-column (col)
  (let* ((bol (save-excursion
		(beginning-of-line)
		(point)))
	 (point-in-whitespace
	  (<= (point) (+ bol (current-indentation))))
	 (blank-line-p
	  (save-excursion
	    (beginning-of-line)
	    (looking-at small-basic-blank-regexp))))

    (cond ((/= col (current-indentation))
	   (save-excursion
	     (beginning-of-line)
	     (back-to-indentation)
	     (delete-region bol (point))
	     (indent-to col))))

    ;; If point was in the whitespace, move back-to-indentation.
    (cond (blank-line-p
	   (end-of-line))
	  (point-in-whitespace
	   (back-to-indentation)))))


(defun small-basic-indent-line ()
  "Indent current line for BASIC"
  (interactive)
  (small-basic-indent-to-column (small-basic-calculate-indent)))


(defun small-basic-function-arg-start (pos endpos)
  (while (and (< pos endpos) (not (char-equal (char-after pos) 40))
              (not (char-equal (char-after pos) 44)))
    (setq pos (+ pos 1)))
  (setq pos (+ pos 1))
  (while (and (< pos endpos) (or
			      (char-equal (char-after pos) 95)
			      (char-equal (char-after pos) 10)
			      (char-equal (char-after pos) 13)
			      (char-equal (char-after pos) 9)
			      (char-equal (char-after pos) 32))
    (setq pos (+ pos 1))))
  (if (< pos endpos)
      pos
    nil))


(defun small-basic-skip-parens (pos endpos)
  (let ((parcount 0))
    (while (and (< pos endpos) (or (> parcount 0)
                                   (char-equal (char-after pos) 40)))
      (if (char-equal (char-after pos) 40)
          (setq parcount (+ parcount 1)))
      (if (char-equal (char-after pos) 41)
          (setq parcount (- parcount 1)))
      (setq pos (+ pos 1)))
    pos))


(defun small-basic-function-arg-end (pos endpos)
  (if (and pos endpos)
      ((lambda ()
         (while (and (<= pos endpos) (not (char-equal (char-after pos) 41))
                     (not (char-equal (char-after pos) 44)))
           (if (char-equal (char-after pos) 40)
               (setq pos (small-basic-skip-parens pos endpos))         
             (setq pos (+ pos 1))))      
         (if (<= pos endpos)
             ((lambda ()
                (setq pos (- pos 1))
                (while (char-equal (char-after pos) 32)
                  (setq pos (- pos 1)))
                (+ pos 1)))
           nil))
       )
    nil))


(defun small-basic-function-get-arguments (pos endpos)
  (let* ((arg-start (small-basic-function-arg-start pos endpos))
         (arg-end (small-basic-function-arg-end arg-start endpos)))
    (if (and arg-start arg-end)
        (cons (buffer-substring arg-start arg-end)
              (small-basic-function-get-arguments arg-end endpos))
      nil)))


(defun small-basic-comment-function-arguments (prefix argument-list)
  (let ((argument (car argument-list))
        (pos (length prefix)))
    (insert prefix)
    (while (< pos 12)
      (insert " ")
      (setq pos (+ pos 1)))
    (insert argument)
    (backward-kill-word 2) ; As type
    (setq pos (+ pos (length argument)))
    (while (< pos 40)
      (insert " ")
      (setq pos (+ pos 1)))
    (insert "\n")
    (if (cdr argument-list)
        (small-basic-comment-function-arguments "'" (cdr argument-list)))))
    
(defun small-basic-string-equal (left right)
  (setq small-basic-previous-buffer (current-buffer))
  (switch-to-buffer "*small-basic-comment-scratch*")
  (insert left)
  (insert " ")
  (insert right)
  (beginning-of-line)
  (downcase-word 1)
  (backward-word 1)
  (setq small-basic-string-equal-retval (current-word))
    (forward-word 1)
    (downcase-word 1)
    (backward-word 1)
    (setq small-basic-string-equal-retval 
	  (string-equal small-basic-string-equal-retval
			(current-word)))
    (kill-buffer "*small-basic-comment-scratch*")
    (switch-to-buffer small-basic-previous-buffer)
  small-basic-string-equal-retval
  )

(defun small-basic-end-of-defun ()
  (interactive)
  (setq end-ps ((lambda () 
		  (end-of-line) 
		  (point)
		  )))
  (beginning-of-line)
  (while (search-forward-regexp "_[ \t]*$" end-ps 1)
    (forward-char 1)
    (setq end-ps ((lambda () 
		  (end-of-line) 
		  (point)
		  )))
    (beginning-of-line)))

(defun small-basic-comment-function ()
  "Adds a SMALL-BASIC function comment header"
  (interactive)
  (let* ((start-pos ((lambda () (beginning-of-line) (point))))
        (end-pos ((lambda () 
		    (small-basic-end-of-defun) 
		    (search-backward-regexp ")[A-Za-z \t_]*$")
		    (point))))
        )   
    (goto-char start-pos)
    (setq case-fold-search t)
    (search-forward "(")
    (backward-char 1)
    (setq arguments (small-basic-function-get-arguments (point) end-pos))
    (goto-char end-pos)
    (forward-word 1)
    (current-word)
    (setq is-a-function (small-basic-string-equal 
			 (current-word) "as"))
    (goto-char start-pos)
    (insert "' ")
    (setq small-basic-return-point-ch (point))
    (insert "\n")
    (cond (arguments
	   (insert "'\n' Parametri:\n'\n")
	   (small-basic-comment-function-arguments "'" arguments))
	  )
    (insert "'\n")
    (if is-a-function (insert "' Ritorna :\n'\n"))
    (goto-char small-basic-return-point-ch)
))

;(global-set-key "\M-\C-q" 'small-basic-comment-function)
