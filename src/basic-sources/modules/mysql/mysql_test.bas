#!/usr/bin/sbasic -m -q

import mysql

h = mysql.connect("localhost", "smallbasic", "root", "p")
? "Handle = "; h
? "DBS    = "; mysql.dbs(h)
? "TABLES = "; mysql.tables(h)
? "Query  = "; mysql.query(h, "SELECT * FROM sbx_counters")
mysql.disconnect h


