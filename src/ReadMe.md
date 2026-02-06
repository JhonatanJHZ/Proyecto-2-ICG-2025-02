# Introducción a la Computación Gráfica.

## Proyecto 2

### Jhonatan Homsany C.I. 30.182.893

## Índice

1. [Introducción](#introducción)
2. [Guía de compilación](#guía-de-compilación)
3. [Guía de uso](#guía-de-uso)
4. [Funcionamiento del programa](#funcionamiento-del-programa)

---

## Introducción.

El presente documento constituye la guía técnica y operativa del sistema desarrollado para la cátedra de Introducción a la Computación Gráfica en la Escuela de Computación de la Facultad de Ciencias de la Universidad Central de Venezuela.

El proyecto consiste en una aplicación de escritorio que permite que los usuarios puedan manipular submallados tridimensionales previamente generados y almacenados en un archivo .obj. Por otro lado, el proyecto ha sido realizado empleando C++ como lenguaje de programación principal utilizando los componentes principales de OpenGL para el despliegue gráfico.

## Guía de compilación.

Este proyecto fue realizado y probado utilizando Visual Studio 2022. Para poder ejecutarlo, es necesario tener instalado MSVC - v143 - VS 2022 C++ x64/x86 build tools (v14.44 - 17.14) como herramienta de construcción de proyectos para así poder compilarlo y ejecutarlo correctamente.

Adicionalmente, la configuración de ejecución se ha establecido en modo $Release$ para plataformas $x64$ usando las siguientes opciones de optimización de C/C++:

<img width="791" height="531" alt="image" src="https://github.com/user-attachments/assets/fb624ae3-7cdd-4f65-819e-d1705a866250" />

<p align="center">
  <i><b>Fig. 1.</b> Configuraciones de compilación y ejecución.</i>
</p>

Una vez establecidas las configuraciones, basta con utilizar el depurador local de Windows en Visual Studio 2022 para ejecutar el proyecto.

## Guía de uso.

- ¿Cómo mover el modelo completo?

El modelo completo puede moverse al hacer click en una ubicación del lienzo que no pertenezca al modelo. Luego, mientras arrastra el mouse manteniendo el click presionado, puede desplazar el modelo hasta soltar el botón.

- ¿Cómo mover un submallado específico?

Un submallado puede moverse al hacer click en un submallado que pertenezca al modelo. Luego, mientras arrastra el mouse manteniendo el click presionado, puede desplazar el submallado hasta soltar el botón.

- ¿Cómo rotar el modelo completo?

El modelo completo puede rotarse al hacer click derecho sobre un punto que no pertenezca al modelo. Luego, mientras arrastra el mouse manteniendo el click presionado, puede rotar el modelo hasta soltar el botón.

- ¿Cómo rotar un submallado específico?

Un submallado puede rotarse al hacer click derecho sobre un submallado que pertenezca al modelo. Luego, mientras arrastra el mouse manteniendo el click presionado, puede rotar el submallado hasta soltar el botón.

- ¿Cómo escalar el modelo completo?

El modelo completo puede escalarse al hacer click sobre un punto que no pertenezca al modelo. Luego, mientras arrastra el mouse manteniendo el click presionado, puede escalar el modelo hasta soltar el botón.

- ¿Cómo escalar un submallado específico?

Un submallado puede escalarse al hacer click sobre un submallado que pertenezca al modelo. Luego, mientras arrastra el mouse manteniendo el click presionado, puede escalar el submallado hasta soltar el botón.

- ¿Cómo rotar la vista simulando la vista de un FPS?

Manteniendo presionado el click central del touchpad (o el botón de la rueda si se utiliza un mouse), puede rotar la vista simulando la vista de un FPS. Luego, mientras arrastra el touchpad (o el mouse) manteniendo el click presionado, puede rotar la vista hasta soltar el botón.

- ¿Cómo hacer zoom al modelo?

El usuario puede acercar o alejar la vista del modelo presionando las teclas UP y DOWN del teclado, respectivamente.

- ¿Cómo desplazar la vista hacia los lados?

El usuario puede desplazar la vista hacia los lados presionando las teclas LEFT y RIGHT del teclado, respectivamente.

- ¿Cómo modificar los atributos de un submallado?

Al presionar un submallado, esto activa diferentes funcionalidades en el panel de control que no se muestran por defecto. Algunas de las funcionalidades que el usuario puede aprovechar son:

1. Cambiar el color del submallado.
2. Mostrar vertices del submallado.
3. Mostrar vectores normales del submallado.
4. Mostrar alambrados del submallado.
5. Modificar color de vertices de un submallado.
6. Mostrar relleno del submallado.
7. Eliminar submallado.

## Funcionamiento del programa.

<img width="1365" height="718" alt="image" src="https://github.com/user-attachments/assets/4896bc18-89ea-453c-9cf2-ed1757c4c3cd" />
<p align="center">
  <i><b>Fig. 3.</b> Manipulación de un modelo 3D de un cerdo.</i>
</p>

<img width="1365" height="719" alt="image" src="https://github.com/user-attachments/assets/06163ba0-6ed3-408e-b647-290e6117d4e0" />
<p align="center">
  <i><b>Fig. 2.</b> Manipulación de un modelo 3D de Super Mario Bros.</i>
</p>

