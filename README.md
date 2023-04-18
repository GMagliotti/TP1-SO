# TP1-SO

Generador de hashes MD5

Para compilar y generar el ejecutable en terminal:
```
    make all
```

Para ejecutar el generador MD5, ingrese:
```
    <path>/md5 <archivos-a-hash>
```
reemplazando archivos-a-hash por los archivos para los cuales desea generar un MD5.

Si quiere ver los hash en tiempo real mientras se generan puede:
1 - Redireccionar la salida de md5 a el ejecutable vista mediante el comando
```
    <path>/md5 <archivos-a-hash> | <path>/vista
```
2 - Ejecutar md5 y utilizar la informacion que este muestra en pantalla para ejecutar vista, mediante el comando:
```
    <path>/vista <cantidad-archivos> <nombre-shm> <nombre-sem>
```
Reemplazando los valores segun la información que corresponda.
Notar que nombre-sem y nombre-shm se distinguen entre ellos con el sufijo _sem y _shm respectivamente
