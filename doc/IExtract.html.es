<!-- -*-HTML-*- -->
<!-- $Id$ -->

<!--
  Anti-Copyright (A) 2003 Markus Schwab (g17m0@lycos.com)

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
  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//-->

<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//ES">
<html>
  <head>
    <title>IExtract - Extractaer informaci&oacute;n desde documentos</title>
    <meta name="description" content="Documentation of the IExtract-utility">
    <meta name="keywords" content="documentacion, docu, IExtract, iextract, utility, utilidad">

    <meta name="DC.Creator" content="Markus Schwab">
    <meta name="DC.Date" content="2003-01-25">
    <meta name="DC.Rights" content="Anticopyright (A) 2003, distributado bajo los condiciones de la GNU Licensia Público">
  </head>

  <body>
    <h1>IExtract - Extractaer informaci&oacute;n desde documentos</h1>
    <hr>

    <p><code>IExtract</code> es una utilidad para extraer los propiedades
      (titulo, autor y commentario) desde varios documentos y presenta los
      en una lista para elaboraci&oacute;n posterior.</p>

    <p>Los documentos siguentes est&aacute;n procesado (distinguido de la
      extensi&oacute;n):</p>

    <dl>
      <dt><b>HTML</b></dt>
      <dd><p>Busca para el texto entre los tags <code>&lt;title&gt;</code> y
          <code>&lt;/title&gt;</code> y the contento de los tags <code>meta</code>
          (tanto en formato HTML 4.0 como "Dublin Core").</p>

        <p>Archivos tienen que tener la extensi&oacute;n <code>*.htm</code>, <code>*.html</code>,
          <code>*.shtm</code>, <code>*.shtml</code> o <code>*.php</code>.</p></dd>

      <dt><b>JPEG</b></dt>
      <dd><p>De hecho no conosco ese formato; pero parece que hay dos formatos por
          los comentarios: Lo simple (como usado de por ejemplo The GIMP) y un
          m&aacute;s complejo (como usado de por ejemplo Windows XP - y todav&iacute;a
          m&aacute;s complejo - de PhotoShop). Ambos formatos est&aacute;n procesado
          (ojal&aacute; correctamente).</p>

        <p>Archivos tiene que tener la extensi&oacute;n <code>*.jpg</code> o
          <code>*.jpeg</code>.</p></dd>

      <dt><b>Documentos de Microsoft Office</b></dt>
      <dd><p>Busca para el contento de la ventana "propiedades".</p>

        <p>Por supuesto ese formato no es documentado (al menos no p&uacute;blico;
          parece que M$ quisiera sacar plata de la documentaci&oacute; tambi&eacute;n
          - o tal vez &iquest;ese formato es tan patetico?) y por eso no puedo
          garantizar que todos los documentos est&aacute;n procesado correctamente.</p>
      </dd>

      <dt><b>MP3</b></dt>
      <dd><p>Extrae el contento of the tag ID3 (versi&oacute;n 1.x). El titulo del
          &aacute;lbum se pone en el campo comentario.</p></dd>

      <dt><b>PDF</b></dt>
      <dd><p>Documentos de PDF contienen un llamado <i>Document Information
            directory</i> (Directorio de informaciones del documento) con varios
          llaves. El contento de aquell directorio est&aacute; extraido (con el
          contento del llave "Subject" (sujecto) como comentario).</p></dd>
      </dd>

      <dt><b>Documentos de StarOffice</b></dt>
      <dd><p>Busca para el contento de la ventana "propiedades".</p>
    </dl>

    <p>La salida puede ser in el formato HTML (una tabla), LaTeX (tabular) o simple
      texto.</p>

    <p>El comportamiento del programa est&aacute; controlado con un archive de
      inicializaci&oacute;n (~/.IExtract para UNICES o %HOMEDRIVE%%HOMEPATH%IExtract.ini
      para Windows). Mira
      <a href="IExtract.html.es#INIfile">Formato de archivos por initializar</a> por
      por m&aacute;s informaci&oacute;n.</p>

    <p>Aquell comportamiento se puede cambiar con especificar un otro archivo o
      opciones al programa.</p>

    <h2>Modo de empleo</h2>

    <pre>   IExtract [OPCIONES] &lt;Archivo(s)&gt;</pre>

    <p>con sentido siguente de los par&aacute;mtros:</p>

    <table>
      <tr valign="top"><td><b>&nbsp;&nbsp;&nbsp;-r,&nbsp;--recursive</b></td>
        <td>Inspectar tambi&eacute; directorios bajos despu&eacute; procesar el
          directorio actual</td></tr>
      <tr><td>&nbsp;</td></tr>

      <tr valign="top"><td>&nbsp;&nbsp;&nbsp;<b>-o,&nbsp;--output=ESTILO</b></td>
        <td>Especifica el estilo de la salida (text, HTML o LaTeX)</td></tr>
      <tr><td>&nbsp;</td></tr>

      <tr valign="top"><td>&nbsp;&nbsp;&nbsp;<b>-f,&nbsp;--format=FORMATO</b></td>
        <td><p>Formato de la salida (default: %n&brvbar;-&brvbar;%t&brvbar;%a&brvbar;%c&brvbar;%d)</p>

          <p>La salida se puede estar en columnas; indicado por un rayo
            (&brvbar;) en el formato.</p>

          <p>El signo del percento (%) indica que la letra siguente tiene significada
              especial:</p>
            <ul>
            <li><b>%a</b> est&aacute; cambiado por el autor</li>
            <li><b>%c</b> est&aacute; cambiado por el commentario</li>
            <li><b>%d</b> est&aacute; cambiado por la fecha y la hora del &uacute;ltimo
              cambio del archivo</li>
            <li><b>%D</b> est&aacute; cambiado por la fecha del &uacute;ltimo
              cambio del archivo</li>
            <li><b>%n</b> est&aacute; cambiado por el nombre del archivo</li>
            <li><b>%N</b> est&aacute; cambiado por el nombre y el camino del archivo</li>
            <li><b>%p</b> est&aacute; cambiado por el camino del archivo</li>
            <li><b>%P</b> est&aacute; cambiado por el camino del archivo en
              estilo UNIX (separado con un slash (/)).</li>
            <li><b>%s</b> est&aacute; cambiado por el tama&ntilde;o del archivo</li>
            <li><b>%S</b> est&aacute; cambiado por el tama&ntilde;o del archivo en
              estilo para seres humanos</li>
            <li><b>%t</b> est&aacute; cambiado por el titulo</li>
            <li><b>%U</b> est&aacute; cambiado por el nombre y camino del archivo en
              estilo UNIX (separado con un slash (/)).</li>
            <li><b>%(LETRAS)</b> est&aacute; cambiado con el primer cambio (de las
              reglas anteriores) que produce un text no vacio (p.e. %(tn) ser&aacute;
              el titulo (si hay) o el nombre del archivo)</li>
          </ul>

          <p>En todos los otros formas el  '%' est&aacute; borrado!</p></td></tr>
      <tr><td>&nbsp;</td></tr>

      <tr valign="top"><td>&nbsp;&nbsp;&nbsp;<b>-T,&nbsp;--title=TITULO</b></td>
        <td><p>Titulo de la salida (escrito tambi&eacute;n si no hay mas salida).</p>

          <p>TITULO especifica los campos de la salida; separada por la raya (&brvbar;);
            cada campo tiene que contener al menos una letra.</p></td></tr>
      <tr><td>&nbsp;</td></tr>

      <tr valign="top"><td>&nbsp;&nbsp;&nbsp;<b>-s,&nbsp;--separate=TEXTO</b></td>
        <td><p>Separar subcarpetas con TEXTO (default: vacio);
            implica recursar por subcarpetas (--recursive)</p>

          <p>El signo del percento (%) indica que la letra siguente tiene significada
              especial:</p>
          <ul>
            <li><b>%e</b> imprima el fin-de-salida por el estilo especifado</li>
            <li><b>%n</b> est&aacute; cambiado por el nombre del la carpeta</li>
            <li><b>%N</b> est&aacute; cambiado por el nombre y el camino de la carpeta</li>
            <li><b>%p</b> est&aacute; cambiado por el camino hasta la carpeta</li>
            <li><b>%P</b> est&aacute; cambiado por el camino hasta la carpeta en
              estilo UNIX (separado con un slash (/)).</li>
            <li><b>%s</b> imprima la empieza-de-salida por el estilo especifado</li>
            <li><b>%U</b> est&aacute; cambiado por el nombre y el camino de la carpeta
              en estilo UNIX (separado con un slash (/)).</li>
          </ul></td></tr>
      <tr><td>&nbsp;</td></tr>

      <tr valign="top"><td>&nbsp;&nbsp;&nbsp;<b>-e,&nbsp;--show-errors</b></td>
        <td>Incluye mesages de error en la salida.</td></tr>
      <tr><td>&nbsp;</td></tr>

      <tr valign="top"><td>&nbsp;&nbsp;&nbsp;<b>-a,&nbsp;--all</b></td>
        <td>Mostra todos los archivos (incluyendo los inconocidos) en la salida.</td></tr>
      <tr><td>&nbsp;</td></tr>

      <tr valign="top"><td>&nbsp;&nbsp;&nbsp;<b>-n,&nbsp;--new=[D&Iacute;AS:]TEXTO</b></td>
        <td><p>Mostra TEXTO en el primer campo por archivos m&aacute;s joven que 
            D&Iacute;AS d&iacute;as (default: 30)</p>

          <p>D&Iacute;AS puede faltar o tener un ap&eacute;ndice de multiplicaci&oacute;n:
            m por 30.</p></td></tr>
      <tr><td>&nbsp;</td></tr>

      <tr valign="top"><td>&nbsp;&nbsp;&nbsp;<b>-i,&nbsp;--include=LISTA</b></td>
        <td><p>Especifica cu&aacute;l archivos deben que est&aacute; inspectado; eso
            puede ser una lista tambi&eacute;; separado por la la letra seperado del
            camino del sistema operativo (dos puntos (:) en Unices; punto y coma (;)
            en Windoze).</p>

          <p>P.e. <code>*.html:*[1-9]*.doc</code></p></td></tr>
      <tr><td>&nbsp;</td></tr>

      <tr valign="top"><td>&nbsp;&nbsp;&nbsp;<b>-x,&nbsp;--exclude=LISTA</b></td>
        <td><p>Especifica cu&aacute;l archivos deben que <b>no</b> est&aacute; inspectado; eso
            puede ser una lista tambi&eacute;; separado por la la letra seperado del
            camino del sistema operativo (dos puntos (:) en Unices; punto y coma (;)
            en Windoze).</p>

          <p>E.g. <code>[[:alpha:]]*.htm?:*.doc</code></p></td></tr>
      <tr><td>&nbsp;</td></tr>

      <tr valign="top"><td>&nbsp;&nbsp;&nbsp;<b>-I,&nbsp;--ini-file=ARCHIVO</b></td>
        <td>Lee m&aacute;s opciones del archivo especifado. Mira
          <a href="IExtract.html.es#INIfile">Formato de archivos por initializar</a> por
          por m&aacute;s informaci&oacute;n.</p></td></tr>
      <tr><td>&nbsp;</td></tr>

      <tr valign="top"><td>&nbsp;&nbsp;&nbsp;<b>-t,&nbsp;--threads=N&Uacute;MERO</b></td>
        <td><p>Pone el n&uacute;mero por los proceses del fondo (threads) para exerminar
            los archivos (adicional al proceso por la busqueada).</p>

          <p>Aquell opci&oacute;n solamente est&aacute; existente, si el programa
            estaba configurado (compilada) con <code>--enable-threads</code> (o
            <code>-DENABLE_THREADS</code>)!</p></td></tr>
      <tr><td>&nbsp;</td></tr>

      <tr valign="top"><td>&nbsp;&nbsp;&nbsp;<b>-V,&nbsp;--version</b></td>
        <td>Mostra informaci&oacute;n sobre la versi&acute;n y termina.</td></tr>
      <tr><td>&nbsp;</td></tr>

      <tr valign="top"><td>&nbsp;&nbsp;&nbsp;<b>-h, -?,&nbsp;--help</b></td>
        <td>Mostra el ayuda y termina.</td></tr>
      <tr><td>&nbsp;</td></tr>

      <tr valign="top"><td>&nbsp;&nbsp;&nbsp;<b>File(s)</b></td>
        <td>Especifica la carpeta y/o los archivos para inspectar..</td></tr>
    </table>

    <h2><a name="INIfile"></a>Formato de archivos por initializar</h2>
    <p>Archivos por initializar tienen el siguente formato (entradas pueden faltar):</p>

    <pre>   [Output]
   Format=FORMATO
   Title=TEXTO
   TextForNewFiles=TEXTO
   MaxAgeForNewFiles=D&Iacute;AS
   DirSeparatorText=TEXTO
   Style=ESTILO</pre>

    <p>Pasan los mismos cambias como con las opciones!</p>

    <hr>
    <address><a href="mailto:g17m0@lycos.com">Envia comentarios</a></address>
<!-- Created: Sat Jan 25 11:45:08 PET 2003 -->
<!-- hhmts start -->
Last modified: Sat Jan 25 20:18:44 PET 2003
<!-- hhmts end -->
     - $Revision$
  </body>
</html>
