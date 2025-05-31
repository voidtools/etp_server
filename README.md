ETP/FTP Server Plugin for [Everything 1.5](https://www.voidtools.com/forum/viewtopic.php?f=12&t=9787)

Allow users to search and access your files from Everything or an FTP client.

ETP is FTP with the [SITE EVERYTHING](#SITE-EVERYTHING) extension.

The [Everything Server](https://github.com/voidtools/everything_server) replaces the ETP/FTP server.

[Download](#download)<br/>
[Install Guide](#Plug-in-Installation)<br/>
[Setup Guide](#Plug-in-Setup)<br/>
[Start an ETP/FTP server](#Start-an-ETPFTP-server)<br/>
[Connect to an ETP server](#Connect-to-an-ETP-server)<br/>
[ETP link types](#ETP-link-types)<br/>
[Username and password](#Username-and-password)<br/>
[Disable file downloading](#Disable-file-downloading)<br/>
[Multiple instances](#Multiple-instances)<br/>
[Connect to multiple servers](#Connect-to-multiple-servers)<br/>
[Index ETP server](#Index-ETP-server)<br/>
[Create a Windows share](#Create-a-Windows-share)<br/>
[Security](#Security)<br/>
[Disable ETP/FTP Server](#Disable-ETPFTP-Server)<br/>
[ETP Client path rewriting](#ETP-Client-path-rewriting)<br/>
[Running an ETP server as a service](#Running-an-ETP-server-as-a-service)<br/>
[Automatically connect to an ETP server](#Automatically-connect-to-an-ETP-server)<br/>
[SITE EVERYTHING](#SITE-EVERYTHING)</br>
[Troubleshooting](#Troubleshooting)<br/>
[See Also](#See-Also)<br/>
<br/><br/><br/>



Download
--------

https://github.com/voidtools/etp_server/releases

https://www.voidtools.com/forum/viewtopic.php?p=35401#etp
<br/><br/><br/>

ETP/FTP Server Options:

![image](https://github.com/user-attachments/assets/cb7472d5-b0c2-4653-b70b-af016d8b2549)
<br/><br/><br/>



Filezilla connected to an ETP/FTP Server:

![image](https://github.com/user-attachments/assets/a2555b5b-838f-4598-8ab1-c961aba3c522)
<br/><br/><br/> 
  


Plug-in Installation
--------------------

To install a plug-in:

*   [Download a plug-in installer](#Download).
*   **Run** the plug-in installer.
*   Click **Add**.

\-or-  
  
To manually install a plug-in:

*   [Download a plug-in zip](#Download) and extract the plug-in dll to your Desktop.
*   Move the extracted plug-in dll to C:\\Program Files\\Everything\\plugins  
    where C:\\Program Files\\Everything is your Everything installation folder.
*   In **Everything**, from the **File** menu, click **Exit**.
*   **Restart Everything**.
<br/><br/><br/>



Plug-in Setup  
-------------

To manage your plug-ins:

*   In **Everything**, from the **Tools** menu, click **Options**.
*   Click the **Plug-ins** tab on the left.
<br/><br/><br/>



Start an ETP/FTP server
-----------------------

To start an ETP/FTP server:

*   On the computer whose files you wish to share, In **_"Everything"_**, from the **Tools** menu, click **Options**.
*   Click the **_ETP/FTP_** tab.
*   Check **Enable ETP/FTP server**.
*   Click **OK**.
<br/><br/><br/>    

  

Please make sure you share each drive on your server as its drive letter.

For example, share C: drive as: **C**

To use custom shares, please see [ETP Client path rewriting](#ETP-Client-path-rewriting).
<br/><br/><br/>

  

Connect to an ETP server
------------------------

To connect to an ETP server:

*   In **"Everything"**, from the **Tools** menu, click **connect to ETP server...**.
*   Type in the **server name** or **IP** for the host.
*   Click OK.
<br/><br/><br/>

  

ETP link types
--------------

There are four link types that change the way the Everything client accesses files on the ETP server.

When connecting to an ETP server you can specify the link type.

<table>
<tr><th>Link type</th><th>Description</th></tr>
<tr><td>C:</td><td>No change, the paths are the same as they are on the server. 
  
This is useful if you index a subst'ed drive on the server and are using mapped network drives on the client.</td></tr>
<tr><td>\\\\Server\\C</td><td>Use Windows shares (this is the default link type). 
  
By default these shares do not exist, you will need to create them on the server if you wish to use this method.</td></tr>
<tr><td>\\\\Server\\C$</td><td>Use the default admin drive shares.</td></tr>
<tr><td>ftp://host/C:</td><td>Use ftp links.</td></tr>
</table>
<br/><br/><br/>

  

Username and password
---------------------

To change the ETP/FTP server username and password:

*   In **_"Everything"_**, from the **Tools** menu, click **Options**.
*   Click the **ETP/FTP Server** tab.
*   Type in a new **username** and **password**.
*   Click **OK**.
<br/><br/><br/>

  

Disable file downloading
------------------------

To disable ETP/FTP file downloading:

*   In **_"Everything"_**, from the **Tools** menu, click **Options**.
*   Click the **ETP/FTP Server** tab.
*   Uncheck **Allow file download**.
*   Click **OK**
<br/><br/><br/>
    
  

Multiple instances
------------------

To index different NTFS volumes for the ETP server, see [Multiple Instances](https://www.voidtools.com/support/everything/multiple_instances).
<br/><br/><br/>

  

Connect to multiple servers
---------------------------

Everything can only connect to one ETP server at a time.

Use the [Everything Server](https://github.com/voidtools/everything_server) to index multiple remote Everything indexes.
<br/><br/><br/>

  

Index ETP server
----------------

Everything cannot index ETP servers.

Use the [Everything Server](https://github.com/voidtools/everything_server) to index a remote Everything index.
<br/><br/><br/>



Create a Windows share
----------------------

Everything works best if you share each drive as a single letter, for example, share your C: drive as C, D: drive as D and so on.

To create a windows share for a single folder, please follow the guide below:

For example, we want to host an ETP/FTP server for one folder:

<code>C:\\share</code>

  

Substitute a drive letter for the local folder c:\\share:

*   From a command prompt, run:<br/>
    <code>subst H: c:\\share</code>
<br/><br/><br/>

  

Setup an index to include only the H: drive.

*   In **_"Everything"_**, from the **Tools** menu, click **Options**.
*   Click the **NTFS** tab.
*   Uncheck **Include in database** for all volumes, except the H: drive.
*   Click **OK**.
<br/><br/><br/>
    
  

Add a share name to c:\\share

*   In **Explorer**, **right click** the **c:\\share** and click **properties**.
*   Click the **Sharing** tab.
*   Click **Advanced Sharing...**.
*   Click **Add**.
*   Type in **H** for the **share name**.
*   Click **OK**.
*   Click **OK**.
*   Click **Close**.

Connect to the ETP/FTP server with \\\\Server\\C links.
<br/><br/><br/>

  

Security
--------

Every file and folder indexed by Everything can be searched and downloaded via the ETP server.

To disable file downloading:

*   In **_Everything_**, from the **Tools** menu, click **Options**.
*   Click the **ETP Server** tab.
*   Uncheck **allow file download**.

See [Disable ETP/FTP Server](#Disable-ETPFTP-Server) to remove the ETP server options and prevent the ETP server from starting.
<br/><br/><br/>

  

Disable ETP/FTP Server
----------------------

To disable the ETP/FTP server:

*   Exit Everything (right click the Everything system tray icon and click Exit)
*   Open your Everything.ini in the same location as your Everything.exe
*   Change the following line:<br/>
    <code>allow\_etp\_server=1</code><br/>
    to:<br/>
    <code>allow\_etp\_server=0</code>
*   Save changes and restart Everything.
<br/><br/><br/>

  

ETP Client path rewriting
-------------------------

ETP clients can rewrite the ETP server paths so they can be accessed from different shares.

For example, rewrite the path D:\\music to \\\\server\\music and "D:\\Install Files" to "\\\\server\\Install Files"

To rewrite the paths on the ETP client.
*   On the **_Everything_** ETP client PC:
*   Completely exit **_Everything_** (Right click the **_Everything_** system tray icon and click Exit)
*   Open your %APPDATA%\\Everything\\Everything.ini
*   Change the following lines:<br/>
    <code>etp\_client\_rewrite\_patterns=</code><br/>
    <code>etp\_client\_rewrite\_substitutions=</code><br/>
    to:<br/>
    <code>etp\_client\_rewrite\_patterns=D:\\music;"D:\\\\Install Files"</code><br/>
    <code>etp\_client\_rewrite\_substitutions=\\\\server\\music;"\\\\\\\\server\\\\Install Files"</code>
*   Save changes and restart Everything.

The pattern must match the path on the server. It is not effected by the link type.
<br/><br/><br/>

  

Running an ETP server as a service
----------------------------------

To run an Everything ETP server as a client service (not to be confused with the Everything service):

*   Copy your Everything.exe to an empty folder.
*   Run Everything.exe as administrator
*   Please make sure [Store settings and data in %APPDATA%\\Everything](https://www.voidtools.com/support/everything/options/#store_settings_and_data_in_appdata_everything) is disabled.
*   Please make sure the [Everything service](https://www.voidtools.com/support/everything/options/#everything_service) is not installed.
*   Setup your [indexes](https://www.voidtools.com/support/everything/options#indexes).
*   Setup the [ETP server settings](https://www.voidtools.com/support/everything/options/#etp_ftp_server).
*   Completely exit Everything (right click the Everything tray icon and click Exit).
*   From a command prompt, navigate to your Everything.exe
*   Run the following command to install the client service:<br/>
    <code>Everything.exe -install-client-service</code><br/>    
    This will install and start the Everything ETP server as a service.
<br/><br/><br/>

  

To uninstall the Everything client service:

*   From a command prompt, navigate to your Everything.exe
*   Run the following command to install the client service:<br/>
    <code>Everything.exe -uninstall-client-service</code>
<br/><br/><br/>

  

Automatically connect to an ETP server
--------------------------------------

To automatically connect to an ETP server when starting Everything:

*   On the **"Everything"** client, from the **Tools** menu, click **Options**.
*   Click the **Home** tab.
*   Change **Index** to: **ETP server**.
*   Set the **ETP server** to: username:password@host:port
*   Set the desired **Link type**.
*   Click **OK**.
<br/><br/><br/>    

  

SITE EVERYTHING
---------------

The Everything ETP/FTP server extends FTP with the command SITE EVERYTHING.

The Everything client uses this extension to request search results.

You can use the SITE command to check if the ETP/FTP server supports the EVERYTHING extension.

<dl>
<dt>SITE EVERYTHING CASE &lt;x&gt;</dt><dd>(Match case if x is nonzero)</dd>
<dt>SITE EVERYTHING WHOLE_WORD &lt;x&gt;</dt><dd>(Match whole words if x is nonzero)</dd>
<dt>SITE EVERYTHING PATH &lt;x&gt;</dt><dd>(Match whole paths if x is nonzero)</dd>
<dt>SITE EVERYTHING DIACRITICS &lt;x&gt;</dt><dd>(Match diacritics if x is nonzero)</dd>
<dt>SITE EVERYTHING REGEX &lt;x&gt;</dt><dd>(perform regex search if x is nonzero)</dd>
<dt>SITE EVERYTHING SEARCH &lt;search-text&gt;</dt><dd>(set the search to search-text)</dd>
<dt>SITE EVERYTHING FILTER_SEARCH abc</dt><dd>(set the secondary search to abc)</dd>
<dt>SITE EVERYTHING FILTER_CASE &lt;x&gt;</dt><dd>(Match case with the secondary search if x is nonzero)</dd>
<dt>SITE EVERYTHING FILTER_WHOLE_WORD &lt;x&gt;</dt><dd>(Match wholewords with the secondary search if x is nonzero)</dd>
<dt>SITE EVERYTHING FILTER_PATH &lt;x&gt;</dt><dd>(Match path with the secondary search if x is nonzero)</dd>
<dt>SITE EVERYTHING FILTER_DIACRITICS &lt;x&gt;</dt><dd>(Match diacritics with the secondary search if x is nonzero)</dd>
<dt>SITE EVERYTHING FILTER_REGEX &lt;x&gt;</dt><dd>(Match regex with the secondary search if x is nonzero)</dd>
<dt>SITE EVERYTHING SORT &lt;x&gt;</dt><dd>(where x is the sort name, see below)</dd>
<dt>SITE EVERYTHING OFFSET &lt;n&gt;</dt><dd>(return results from the nth item)</dd>
<dt>SITE EVERYTHING COUNT &lt;x&gt;</dt><dd>(return no more than x results)</dd>
<dt>SITE EVERYTHING SIZE_COLUMN &lt;x&gt;</dt><dd>(return the result's size if x is nonzero)</dd>
<dt>SITE EVERYTHING DATE_CREATED_COLUMN &lt;x&gt;</dt><dd>(return the result's creation date if x is nonzero)</dd>
<dt>SITE EVERYTHING DATE_MODIFIED_COLUMN &lt;x&gt;</dt><dd>(return the result's modified date if x is nonzero)</dd>
<dt>SITE EVERYTHING ATTRIBUTES_COLUMN &lt;x&gt;</dt><dd>(return the result's attributes if x is nonzero)</dd>
<dt>SITE EVERYTHING PATH_COLUMN &lt;x&gt;</dt><dd>(return the result's path if x is nonzero)</dd>
<dt>SITE EVERYTHING FILE_LIST_FILENAME_COLUMN &lt;x&gt;</dt><dd>(return the result's file list filename if x is nonzero)</dd>
<dt>SITE EVERYTHING QUERY</dt><dd>executes the query with the current search state</dd>
</dl>
<br/><br/><br/>



Default state:
<table>
<tr><td>EVERYTHING CASE</td><td>0</td></tr>
<tr><td>EVERYTHING WHOLE_WORD</td><td>0</td></tr>
<tr><td>EVERYTHING PATH</td><td>0</td></tr>
<tr><td>EVERYTHING DIACRITICS</td><td>0</td></tr>
<tr><td>EVERYTHING REGEX</td><td>0</td></tr>
<tr><td>EVERYTHING SEAR</td><td>H</td></tr>
<tr><td>EVERYTHING FILTER_SEARCH</td><td></td></tr>
<tr><td>EVERYTHING FILTER_CASE</td><td>0</td></tr>
<tr><td>EVERYTHING FILTER_WHOLE_WORD</td><td>0</td></tr>
<tr><td>EVERYTHING FILTER_PATH</td><td>0</td></tr>
<tr><td>EVERYTHING FILTER_DIACRITICS</td><td>0</td></tr>
<tr><td>EVERYTHING FILTER_REGEX</td><td>0</td></tr>
<tr><td>EVERYTHING SORT</td><td>0</td></tr>
<tr><td>EVERYTHING OFFSET</td><td>0</td></tr>
<tr><td>EVERYTHING COUNT</td><td>0</td></tr>
<tr><td>EVERYTHING SIZE_COLUMN</td><td>0</td></tr>
<tr><td>EVERYTHING DATE_CREATED_COLUMN</td><td>0</td></tr>
<tr><td>EVERYTHING DATE_MODIFIED_COLUMN</td><td>0</td></tr>
<tr><td>EVERYTHING ATTRIBUTES_COLUMN</td><td>0</td></tr>
<tr><td>EVERYTHING PATH_COLUMN</td><td>0</td></tr>
<tr><td>EVERYTHING FILE_LIST_FILENAME_COLUMN</td><td>0</td></tr>
</table>
<br/><br/><br/>



For example, to find the first 100 items that contain abc:

<code>EVERYTHING SEARCH abc
EVERYTHING COUNT 100
EVERYTHING PATH_COLUMN 1
EVERYTHING QUERY</code>
<br/><br/><br/>



Sort names

*   NAME_ASCENDING
*   NAME_DESCENDING
*   PATH_ASCENDING
*   PATH_DESCENDING
*   SIZE_ASCENDING
*   SIZE_DESCENDING
*   EXTENSION_ASCENDING
*   EXTENSION_DESCENDING
*   DATE_CREATED_ASCENDING
*   DATE_CREATED_DESCENDING
*   DATE_MODIFIED_ASCENDING
*   DATE_MODIFIED_DESCENDING
*   ATTRIBUTES_ASCENDING
*   ATTRIBUTES_DESCENDING
*   FILE_LIST_FILENAME_ASCENDING
*   FILE_LIST_FILENAME_DESCENDING
<br/><br/><br/>



ETP also supports the <code>EVERYTHING</code> command.
Use <code>FEAT</code> to check if the EVERYTHING command is supported.
The <code>EVERYTHING</code> command is the same as the <code>SITE EVERYTHING</code> command.
<br/><br/><br/>



Troubleshooting
---------------

Unable to start ETP server: bind failed 10048

Please make sure no FTP servers are already running on port 21 or use a different ETP server port.

  

To change the ETP server port:

*   In **"Everything"**, from the **Tools** menu, click **Options**.
*   Click the **ETP/FTP Server** tab.
*   Change the **port** to: **2121**
*   Click **OK**.

Please match the same port number when connecting to the ETP server.
<br/><br/><br/>



See also
--------

*   [Multiple Instances](https://www.voidtools.com/support/everything/multiple_instances)
*   https://www.voidtools.com/support/everything/etp/
*   [ETP/FTP Server Help](https://www.voidtools.com/forum/viewtopic.php?p=5731)
*   [Everything Server](https://github.com/voidtools/everything_server)
*   [Everything 1.5 Plugins - ETP/FTP Server](https://www.voidtools.com/forum/viewtopic.php?p=35401#etp)
*   [Everything 1.5 Plugin SDK](https://www.voidtools.com/forum/viewtopic.php?t=16535)
