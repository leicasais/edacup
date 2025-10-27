# Simulación de robots EDACup 2025

## Compilar un proyecto de ejemplo

1. Abre el proyecto `move_sample` (ubicado en la carpeta `move_sample`) en tu entorno de desarrollo.

2. Compila el proyecto.

3. Verifica la ubicación del binario generado (en Windows, suele estar en `build\debug\move_sample.exe`).

## Instalar el simulador

1. Descarga e instala Node.js:

  * Visita https://nodejs.org/ y sigue las instruacciones para instalar Node.js.

2. Instala las dependencias:

 * Abre una terminal y navega a la carpeta que contiene el archivo `README.md`.
 * Ejecuta los siguientes comandos para instalar los paquetes necesarios:

        npm install ws
        npm install "@roamhq/wrtc"

3. Configura el archivo de configuración:

  * Abre el archivo `edacup.json` en un editor de texto.
  * Configura el campo `accessToken` con el token correspondiente a tu grupo.
  * Establece el campo `team1Binary` con la ruta al binario generado en el paso anterior.
  * (Opcional) Configura `team2Binary` con la misma ruta si deseas controlar ambos equipos con el binario de ejemplo.

## Ejecutar el simulador

1. En la línea de comandos, navega a la carpeta que contiene el archivo `README.md`.

2. Inicia el simulador con el siguiente comando:

    node edacup/edacup.mjs

3. Abre un navegador y accede a https://www.openreality.app/ para visualizar la simulación.

4. Para reanudar el juego, usa el comando "p" en la terminal simulador. Puedes detener el juego con el comando "s", y reiniciar los robots con el comando "r".
