<!-- -*-HTML-*- -->
<!-- $Id$ -->

<!--
  Copyright (C) 2003 - 2005 Markus Schwab (g17m0@lycos.com)

  This is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public
  License along with this library; if not, write to the Free
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
-->

<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//ES"
"http://www.w3.org/TR/html4/strict.dtd">
<html>
  <head>
    <title>IExtract - Extraer informaciones de documentos</title>
    <meta name="description" content="Documentation of the IExtract-utility">
    <meta name="keywords" content="documentacion, docu, IExtract, iextract, utility, utilidad">

    <meta name="DC.Creator" content="Markus Schwab">
    <meta name="DC.Date" content="2003-01-25">
    <meta name="DC.Rights" content="Copyright (C) 2003 - 2006; distribuido bajo las condiciones de la GNU Licensia Público">

    <style type="text/css">
      <!--
      td.selected { vertical-align:middle; background-color:#5BAAEA; }
      td.unselected { vertical-align:middle; background-color:lightgrey; }

      /* Colors for links */
      a:link { color:darkblue; text-decoration:underline; }
      a:visited { color:black; text-decoration:underline; }
      -->
    </style>
  </head>

  <body>
    <h1>IExtract - Extraer informaciones desde documentos</h1>
    <hr>

    <table width="100%" cellpadding="5" cellspacing="0" >
      <tr valign=top><td class="unselected"><a href="IExtract.html.en">
            English&nbsp;version</a></td>
        <td>&nbsp;</td>
        <td rowspan="3">
          <p>&Eacute;ste programa se distribuye bajo las condiciones de la
            <a href="http://www.gnu.org">GNU</a> Licensia General
            P&uacute;blico; con la esperanza de que sea &uacute;til,
            pero SIN NINGUNA GARANT&Iacute;A, ni siquiera la
            garant&iacute;a impl&iacute;cita de COMERCIABILIDAD o
            CONVENIENCIA PARA UN PROP&Oacute;SITO
            PARTICULAR.</p></tr>
      <tr><td class="unselected"><a href="IExtract.html.de">Deutsche&nbsp;Version</a></td>
        <td>&nbsp;</td></tr>
      <tr><td class="selected">Versi&oacute;n&nbsp;en&nbsp;castellano</td>
        <td>&nbsp;</td></tr>
    </table>
    <hr size=1 noshade>

    <p><code>IExtract</code> es una utilidad para extraer
            las propiedades (t&iacute;tulo, autor y comentario) de varios
            tipos de documentos y presentarlos en una lista para elaboraci&oacute;n
            posterior.</p>

    <p>Los documentos siguentes est&aacute;n procesados:</p>

    <dl>
      <dt><b>HTML</b></dt>
      <dd><p>Busca para el texto entre los tags <code>&lt;title&gt;</code> y
          <code>&lt;/title&gt;</code> y el contenido de los tags <code>meta</code>
          (tanto en formato HTML 4.0 como "Dublin Core").</p></dd>

      <dt><b>PNG</b></dt>
      <dd><p>Imagenes del formato PNG pueden contener entradas de texto
          en el formato <em>identificador</em> - <em>valor</em>.</p>

	<p>El contenido (incomprimido) de los identificadores
	  <em>Title</em>, <em>Author</em> y <em>Description</em>
	  est&aacute; extraido.</p></dd>

      <dt><b>GIF</b></dt>
      <dd><p>Imagenes del formato GIF pueden contener entradas de comentario.
          Esos entradas est&aacute;n extraidos en el campo
	  "comentario". Las entradas por el autor y el t&iacute;tulo
	  hacen falta.</p></dd>

      <dt><b>JPEG</b></dt>
      <dd><p>El programa reconoce comentarios despu&eacute;s de un
          "marcador de comentarios" (0xFFFE), en un "marcador APP1 Exif"
	  (como le usa Windows XP) y en un "marcador APPD" (como le usan
	  unas versiones de PhotoShop).</p></dd>

      <dt><b>Documentos de Microsoft Office</b></dt>
      <dd><p>Busca para el contenido de la ventana "propiedades".</p>

        <p>Gracias al proyecto de Apache Jakarta POI y su
	  documentaci&oacute;n del "formato OLE2 Document Compound"
	  (como lo usa el MS Office) todos los documentos deben estar
	  le&iacute;ble (tal vez con la excepci&oacute;n de documentos
	  m&aacute;s grande que 6.8 MB).</p></dd>

      <dt><b>MP3</b></dt>
      <dd><p>Extrae el contenido del tag ID3 (versi&oacute;n 1.x y
          2.x). El t&iacute;tulo del &aacute;lbun est&aacute; puesto
          en el campo comentario.</p></dd>

      <dt><b>OGG</b></dt>
      <dd><p>Extrae el contenido del "comment header" (cabeza de comentario).
          El t&iacute;tulo del &aacute;lbun est&aacute; puesto
          en el campo comentario.</p></dd>

      <dt><b>PDF</b></dt>
      <dd><p>Documentos de PDF contienen un llamado <i>Document Information
            directory</i> (Directorio de informaciones del documento) con varias
          llaves. El contenido de aquel directorio est&aacute; extraido (con el
          contenido de la llave "Subject" (sujecto) como comentario).</p>

        <p>&iexcl;Datos codificados no est&aacute;n decifrados!</p>
      </dd>

      <dt><b>Documentos de StarOffice</b></dt>
      <dd><p>Busca para el contenido de la ventana "propiedades".</p>

      <dt><b>Documentos de OpenOffice</b></dt>
      <dd><p>Busca para el contenido de la ventana "propiedades".</p>

      <dt><b>Documentos de Abiword</b></dt>
      <dd><p>Busca para el contenido de la ventana "propiedades".</p>

      <dt><b>Documentos en formato RTF (Rich Text Format)</b></dt>
      <dd><p>Busca para el contenido del bloque "info".</p>
    </dl>

    <p>La salida puede ser en el formato HTML (una tabla), XML (XHTML por
      defecto), LaTeX (tabular) o simple texto (o legible (separado por
      espacio) o legible por m&aacute;quinas (en cuotas, separado por comas)).
      Nota que letras especiales en la informaci&oacute;n extraido est&aacute;
      cambiado.</p>

    <p>El comportamiento del programa est&aacute; controlado con un archivo de
      iniciaci&oacute;n (~/.IExtract para UNICES o %HOMEDRIVE%%HOMEPATH%IExtract.ini
      para Windows). Mira
      <a href="IExtract.html.es#INIfile">Formato de archivos por iniciar</a> por
      por m&aacute;s informaci&oacute;n.</p>

    <p>Aquel comportamiento se puede cambiar con especificar un otro archivo o
      opciones al programa.</p>

    <h2>Modo de empleo</h2>

    <pre>   IExtract [OPCIONES] &lt;Archivo(s)&gt;</pre>

    <p><a name="Fileformat"></a>Los archivos busqueados pueden
      contener los comodines tipicales de UNIX (estrella (*) por cada
      numero de todas las letras y, el signo de interrogación (?) por
      cualquiera letra sola y letras especificas en chorchetes ([) y
      (]), o detalladolos, especifacadolos por sus l&iacute;mites
      (separado por un gui&oacute;n) o sus clases (entre '[:' y
      ':]'). Por invertir esa selecci&oacute;n usa la caret (^) o el
      signo de exclamaci&oacute;n (!) en la primera posici&oacute;). Eso
      tambi&eacute;n vale por la versi&oacute;n de Windows.</p>

    <p>Por ejemplo:</p>

    <dl>
      <dt><b>*.mp3</b></dt>
      <dd>Inspecta solamente archivos MP3</dd>
      <dt><b>[A-Za-z]*</b></dt>
      <dd>Inspecta ficheros que comienzan con una letra.</dd>
      <dt><b>[^[:alnum:]]*</b></dt>
      <dd>Inspecta ficheros que no comienzan con una letra o un n&uacute;mero.</dd>
      <dt><b>???.txt</b></dt>
      <dd>Inspecta ficheros de texto que tienen 3 letras.</dd>
    </dl>

    <h3>Opciones</h3>
    <p>Las opciones siguentes est&aacute;n reconocido del programa (opciones cortes
      se puede combinar juntos, si la opci&oacute;n larga necesita un argumento,
      tambi&eacute;n lo necesito su correspondencia larga. Opciones larges pueden
      estar acortado hasta est&aacute;n unique):</p>

    <table>
      <tr valign="top"><td><b>&nbsp;&nbsp;&nbsp;-r,&nbsp;--recursive</b></td>
        <td>Investigar tambi&eacute;n directorios bajos despu&eacute;s procesar el
          directorio actual</td></tr>
      <tr><td>&nbsp;</td></tr>

      <tr valign="top"><td>&nbsp;&nbsp;&nbsp;<b>-o,&nbsp;--output=ESTILO</b></td>
        <td>Especifica el estilo de la salida (text, quoted, HTML o LaTeX).</td></tr>
      <tr><td>&nbsp;</td></tr>

      <tr valign="top"><td>&nbsp;&nbsp;&nbsp;<b>-f,&nbsp;--format=FORMATO</b></td>
        <td>Formato de la salida (por defecto: %n&brvbar;-&brvbar;%t&brvbar;%a&brvbar;%c&brvbar;%d)

          <p>La salida se puede estar en columnas; indicado por un rayo
            (&brvbar;) en el formato.</p>

          <p>El signo del porciento (%) indica que la letra siguente tiene significado
              especial:</p>
            <ul>
            <li><b>%a</b> est&aacute; cambiado por el autor</li>
            <li><b>%c</b> est&aacute; cambiado por el comentario</li>
            <li><b>%d</b> est&aacute; cambiado por la fecha y la hora del &uacute;ltimo
              cambio del archivo</li>
            <li><b>%D</b> est&aacute; cambiado por la fecha del &uacute;ltimo
              cambio del archivo</li>
            <li><b>%e</b> est&aacute; cambiado por la extensi&oacute;n del archivo</li>
            <li><b>%N</b> est&aacute; cambiado por el nombre del archivo sin extensi&oacute;n</li>
            <li><b>%n</b> est&aacute; cambiado por el nombre del archivo</li>
            <li><b>%N</b> est&aacute; cambiado por el nombre y el camino del archivo</li>
            <li><b>%p</b> est&aacute; cambiado por el camino del archivo</li>
            <li><b>%P</b> est&aacute; cambiado por el camino del archivo en
              estilo UNIX (separado por un slash (/)).</li>
            <li><b>%s</b> est&aacute; cambiado por el tama&ntilde;o del archivo</li>
            <li><b>%S</b> est&aacute; cambiado por el tama&ntilde;o del archivo en
              estilo para seres humanos</li>
            <li><b>%t</b> est&aacute; cambiado por el t&iacute;tulo</li>
            <li><b>%U</b> est&aacute; cambiado por el nombre y camino del archivo en
              estilo UNIX (separado por un slash (/)).</li>
            <li><b>%(LETRAS)</b> est&aacute; cambiado por el primer cambio (de las
              reglas anteriores) que produce un text no vacio (p.e. %(tn) ser&aacute;
              el t&iacute;tulo (si hay) o el nombre del archivo)</li>
            <li><b>%*LETRA</b> cambia la significancia de LETRA un pocito. Para
              cambios del nombre del archivo causa adicional el cambio de
              letras especiales (cuales dependenden del modo de la salida) y
              por los demas suprime ese cambio adicional.</li>
          </ul>

          <p>&iexcl;En todas las otras formas el  '%' est&aacute; borrado!</p></td></tr>
      <tr><td>&nbsp;</td></tr>

      <tr valign="top"><td>&nbsp;&nbsp;&nbsp;<b>-T,&nbsp;--title=T&Iacute;TULO</b></td>
        <td>T&iacute;tulo de la salida (escrito tambi&eacute;n si no hay mas salida).

          <p>T&Iacute;TULO especifica los campos de la salida; separada por la raya (&brvbar;);
            cada campo tiene que contener al menos una letra.</p></td></tr>
      <tr><td>&nbsp;</td></tr>

      <tr valign="top"><td>&nbsp;&nbsp;&nbsp;<b>-s,&nbsp;--separate=TEXTO</b></td>
        <td>Separar subcarpetas por TEXTO (por defecto: vacio);
          implica recursar por subcarpetas (--recursive).

          <p>El signo del porciento (%) indica que la letra siguente tiene significado
              especial:</p>
          <ul>
            <li><b>%e</b> imprima el fin-de-salida por el estilo especifado</li>
            <li><b>%n</b> est&aacute; cambiado por el nombre del la carpeta</li>
            <li><b>%N</b> est&aacute; cambiado por el nombre y el camino de la carpeta</li>
            <li><b>%p</b> est&aacute; cambiado por el camino hasta la carpeta</li>
            <li><b>%P</b> est&aacute; cambiado por el camino hasta la carpeta en
              estilo UNIX (separado por un slash (/)).</li>
            <li><b>%s</b> imprima la empieza-de-salida por el estilo especifado</li>
            <li><b>%U</b> est&aacute; cambiado por el nombre y el camino de la carpeta
              en estilo UNIX (separado por un slash (/)).</li>
          </ul></td></tr>
      <tr><td>&nbsp;</td></tr>

      <tr valign="top"><td>&nbsp;&nbsp;&nbsp;<b>-p,&nbsp;--prepend=TEXTO</b></td>
        <td>Especifica un texto, cual est&aacute; mostrado antes cualquiera
          salida (p.ej. por un encabezamiento).</td></tr>
      <tr><td>&nbsp;</td></tr>

      <tr valign="top"><td>&nbsp;&nbsp;&nbsp;<b>-P,&nbsp;--pre-file=ARCHIVO</b></td>
        <td>Especifica un archivo, cual contenido est&aacute; mostrado antes
          cualquiera salida (p.ej. por un encabezamiento).</td></tr>
      <tr><td>&nbsp;</td></tr>

      <tr valign="top"><td>&nbsp;&nbsp;&nbsp;<b>-a,&nbsp;--append=TEXTO</b></td>
        <td>Especifica un texto, cual est&aacute; mostrado despu&eacute;s
          cualquiera salida (p.ej. por un pie de p&aacute;gina).</td></tr>
      <tr><td>&nbsp;</td></tr>

      <tr valign="top"><td>&nbsp;&nbsp;&nbsp;<b>-A,&nbsp;--app-file=ARCHIVO</b></td>
        <td>Especifica un archivo, cual contenido est&aacute; mostrado despu&eacute;s
          cualquiera salida (p.ej. por un pie de p&aacute;gina).</td></tr>
      <tr><td>&nbsp;</td></tr>

      <tr valign="top"><td>&nbsp;&nbsp;&nbsp;<b>-e,&nbsp;--show-errors</b></td>
        <td>Incluye mensajes de error en la salida.</td></tr>
      <tr><td>&nbsp;</td></tr>

      <tr valign="top"><td>&nbsp;&nbsp;&nbsp;<b>-u,&nbsp;--add-unknown</b></td>
        <td>Muestra todos los archivos (incluyendo los inconocidos) en la salida.</td></tr>
      <tr><td>&nbsp;</td></tr>

      <tr valign="top"><td>&nbsp;&nbsp;&nbsp;<b>-n,&nbsp;--new=[D&Iacute;AS:]TEXTO</b></td>
        <td>Muestra TEXTO en el primer campo por archivos m&aacute;s jovenes que
          D&Iacute;AS d&iacute;as (por defecto: 30).

          <p>D&Iacute;AS puede faltar o tener un ap&eacute;ndice de multiplicaci&oacute;n:
            m por 30.</p></td></tr>
      <tr><td>&nbsp;</td></tr>

      <tr valign="top"><td>&nbsp;&nbsp;&nbsp;<b>-i,&nbsp;--include=LISTA</b></td>
        <td>Especifica cu&aacute;les archivos deben estar investigados;
          eso puede tambi&eacute;n ser una lista; separado por la la
          letra de separaci&oacute;n del camino del sistema operativo
          (dos puntos (:) en Unices; punto y coma (;) en Windows).

          <p>Los archivos pueden contener los comodines tipicales de UNIX.
            <a href="IExtract.html.de#Fileformat">Detalles</a> se encuentra
            m&aacute;s arriba en el documento.</p></td></tr>
      <tr><td>&nbsp;</td></tr>

      <tr valign="top"><td>&nbsp;&nbsp;&nbsp;<b>-x,&nbsp;--exclude=LISTA</b></td>
        <td>Especifica cu&aacute;les archivos <b>no</b> deben estar
          investigados; eso puede tambi&eacute;n ser una lista; separado por la la
          letra de separaci&oacute;n del camino del sistema operativo (dos puntos (:) en Unices;
          punto y coma (;) en Windows).

          <p>Los archivos pueden contener los comodines tipicales de UNIX.
            <a href="IExtract.html.de#Fileformat">Detalles</a> se encuentra
            m&aacute;s arriba en el documento.</p></td></tr>
      <tr><td>&nbsp;</td></tr>

      <tr valign="top"><td>&nbsp;&nbsp;&nbsp;<b>-I,&nbsp;--ini-file=ARCHIVO</b></td>
        <td>Lee m&aacute;s opciones del archivo especificado. Mira
          <a href="IExtract.html.es#INIfile">Formato de archivos por iniciar</a>
          por m&aacute;s informaci&oacute;n.</p></td></tr>
      <tr><td>&nbsp;</td></tr>

      <tr valign="top"><td>&nbsp;&nbsp;&nbsp;<b>-t,&nbsp;--threads=N&Uacute;MERO</b></td>
        <td>Pone el n&uacute;mero por los proceses del fondo (threads) para examinar
          los archivos (adicional al proceso por la b&uacute;squeda).

          <p>Esa opci&oacute;n es solamente disponible, si el programa
            estaba configurado (compilado) con <code>--enable-threads</code> (o
            <code>-DENABLE_THREADS</code>)!</p></td></tr>
      <tr><td>&nbsp;</td></tr>

      <tr valign="top"><td>&nbsp;&nbsp;&nbsp;<b>-S,&nbsp;--sort</b></td>
        <td>Sortea los fichereos alfabeticamente.</td></tr>
      <tr><td>&nbsp;</td></tr>

        <tr valign="top"><td>&nbsp;&nbsp;&nbsp;<b>-M,&nbsp;--mode=[MODO]</b></td>
        <td>Specifica, como se determina el tipo del archivo. Modos posibles
          son:

	  <dl>
	    <dt><b>Ext</b></dt><dd>De la &uacute;ltima extensi&oacute;n del
               nombre.</dd>
	    <dt><b>AllExt</b></dt><dd>De la &uacute;ltima extensi&oacute;n
              conocida del nombre.</dd>
	    <dt><b>Content</b></dt><dd>Del contenido del archivo. Eso busca
	      para identificaciones caracteristicas de los diferentes
	      tipos de archivos.</dd>
	  </dl></td></tr>
      <tr><td>&nbsp;</td></tr>

      <tr valign="top"><td>&nbsp;&nbsp;&nbsp;<b>-V,&nbsp;--version</b></td>
        <td>Muestra informaci&oacute;n sobre la versi&oacute;n y termina.</td></tr>
      <tr><td>&nbsp;</td></tr>

      <tr valign="top"><td>&nbsp;&nbsp;&nbsp;<b>-h, -?,&nbsp;--help</b></td>
        <td>Muestra la ayuda y termina.</td></tr>
      <tr><td>&nbsp;</td></tr>

      <tr valign="top"><td>&nbsp;&nbsp;&nbsp;<b>Archivo(s)</b></td>
        <td>Especifica la carpeta y/o los archivos para investigar.</td></tr>
    </table>

    <p>Los opciones <i>append</i> y <i>app-file</i> (o equivalente
      <i>prepend</i> y <i>pre-file</i>) pueden ser repetidos. Cada
      opci&oacute;n nuevo a&ntilde;ade su texto a los anteriores.</p>

    <h2><a name="INIfile"></a>Formato de archivos por iniciar</h2>
    <p>Los archivos por iniciar tienen el siguente formato (entradas pueden faltar):</p>

    <pre>   [Output]
   Format=FORMATO
   Title=TEXTO
   TextForNewFiles=TEXTO
   MaxAgeForNewFiles=D&Iacute;AS
   DirSeparatorText=T&Iacute;TULO
   Style=ESTILO
   SortFiles=1</pre>

    <p>&iexcl;Pasan los mismos cambios como con las opciones!</p>

    <hr>
    <address><a href="mailto:g17m0@lycos.com">Envia comentarios</a></address>
  </body>
</html>
