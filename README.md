# TP1-SO

Generador de hashes MD5

Para compilar y generar el ejecutable: 'make all' en terminal

Para ejecutar el generador MD5, ingrese:
'./md5 <archivos-a-hash>'
sin comillas y reemplazando <archivos-a-hash> por los archivos para los cuales desea generar un MD5.

Si quiere ver los hash en tiempo real mientras se generan puede:
1 - Redireccionar la salida de md5 a el ejecutable vista mediante el comando
'<path>/md5 <archivos-a-hash> | <path>/vista'

2 - Ejecutar md5 y utilizar la informacion que este muestra en pantalla para ejecutar vista, mediante el comando:
'/vista <cantidad-archivos> <nombre-shm> <nombre-sem>'
Reemplazando los valores entre menor/mayor segun corresponda.
Notar que nombre-sem y nombre-shm se distinguen entre ellos con el sufijo _sem y _shm respectivamente
