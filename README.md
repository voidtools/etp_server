ETP/FTP Server Plugin for [Everything 1.5](https://www.voidtools.com/forum/viewtopic.php?f=12&t=9787)

Allow users to search and access your files from Everything or an FTP client.

ETP is FTP with the [SITE EVERYTHING](#SITE-EVERYTHING) extension.

[Download](#download)<br/>
[Install Guide](#Plug-in-Installation)<br/>
[Setup Guide](#Plug-in-Setup)<br/>
[Start an ETP/FTP server](#Start-an-ETP/FTP-server)<br/>
[Connect to an ETP server](#Connect-to-an-ETP-server)<br/>
[ETP link types](#ETP-link-types)<br/>
[Username and password](#Username-and-password)<br/>
[Disable file downloading](#Disable-file-downloading)<br/>
[Different indexes](#Different-indexes)<br/>
[Create a Windows share](#Create-a-Windows-share)<br/>
[SITE EVERYTHING](#SITE-EVERYTHING)</br>
[Security](#Security)<br/>
[Disable ETP/FTP Server](#Disable-ETP/FTP-Server)<br/>
[ETP Client path rewriting](#ETP-Client-path-rewriting)<br/>
[Running an ETP server as a service](#Running-an-ETP-server-as-a-service)<br/>
[Automatically connect to an ETP server](#Automatically-connect-to-an-ETP-server)<br/>
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

To use custom shares, please see [ETP path rewriting](/support/everything/etp/#etp_client_path_rewriting).
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
    
  

Different indexes
-----------------

To index different NTFS volumes for the ETP server, see [Multiple Instances](/support/everything/multiple_instances).
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

  

SITE EVERYTHING
---------------

The Everything ETP/FTP server extends FTP with the command SITE EVERYTHING.

The Everything client uses this extension to request search results.

You can use the SITE command to check if the ETP/FTP server supports the EVERYTHING extension.

EVERYTHING CASE x (Match case if x is nonzero)
EVERYTHING WHOLE_WORD x (Match whole words if x is nonzero)
EVERYTHING PATH x (Match whole paths if x is nonzero)
EVERYTHING DIACRITICS x (Match diacritics if x is nonzero)
EVERYTHING REGEX x (perform regex search if x is nonzero)
EVERYTHING SEARCH abc (set the search to abc)
EVERYTHING FILTER_SEARCH abc (set the secondary search to abc)
EVERYTHING FILTER_CASE x (Match case with the secondary search if x is nonzero)
EVERYTHING FILTER_WHOLE_WORD x (Match wholewords with the secondary search if x is nonzero)
EVERYTHING FILTER_PATH x (Match path with the secondary search if x is nonzero)
EVERYTHING FILTER_DIACRITICS x (Match diacritics with the secondary search if x is nonzero)
EVERYTHING FILTER_REGEX x (Match regex with the secondary search if x is nonzero)
EVERYTHING SORT x (where x is the sort name, see below)
EVERYTHING OFFSET n (return results from the nth item)
EVERYTHING COUNT x (return no more than x results)
EVERYTHING SIZE_COLUMN x (return the result's size if x is nonzero)
EVERYTHING DATE_CREATED_COLUMN x (return the result's creation date if x is nonzero)
EVERYTHING DATE_MODIFIED_COLUMN x (return the result's modified date if x is nonzero)
EVERYTHING ATTRIBUTES_COLUMN x (return the result's attributes if x is nonzero)
EVERYTHING PATH_COLUMN x (return the result's path if x is nonzero)
EVERYTHING FILE_LIST_FILENAME_COLUMN x (return the result's file list filename if x is nonzero)
EVERYTHING QUERY (executes the query with the above settings)

Default values:
EVERYTHING CASE 0
EVERYTHING WHOLE_WORD 0
EVERYTHING PATH 0
EVERYTHING DIACRITICS 0
EVERYTHING REGEX 0
EVERYTHING SEARCH
EVERYTHING FILTER_SEARCH
EVERYTHING FILTER_CASE 0
EVERYTHING FILTER_WHOLE_WORD 0
EVERYTHING FILTER_PATH 0
EVERYTHING FILTER_DIACRITICS 0
EVERYTHING FILTER_REGEX 0
EVERYTHING SORT 0
EVERYTHING OFFSET 0
EVERYTHING COUNT 0
EVERYTHING SIZE_COLUMN 0
EVERYTHING DATE_CREATED_COLUMN 0
EVERYTHING DATE_MODIFIED_COLUMN 0
EVERYTHING ATTRIBUTES_COLUMN 0
EVERYTHING PATH_COLUMN 0
EVERYTHING FILE_LIST_FILENAME_COLUMN 0

For example, find the first 100 items that contain abc:
EVERYTHING SEARCH abc
EVERYTHING COUNT 100
EVERYTHING PATH_COLUMN 1
EVERYTHING QUERY

Sort names
NAME_ASCENDING
NAME_DESCENDING
PATH_ASCENDING
PATH_DESCENDING
SIZE_ASCENDING
SIZE_DESCENDING
EXTENSION_ASCENDING
EXTENSION_DESCENDING
DATE_CREATED_ASCENDING
DATE_CREATED_DESCENDING
DATE_MODIFIED_ASCENDING
DATE_MODIFIED_DESCENDING
ATTRIBUTES_ASCENDING
ATTRIBUTES_DESCENDING
FILE_LIST_FILENAME_ASCENDING
FILE_LIST_FILENAME_DESCENDING



Security
--------

Every file and folder indexed by Everything can be searched and downloaded via the ETP server.

To disable file downloading:

*   In **_Everything_**, from the **Tools** menu, click **Options**.
*   Click the **ETP Server** tab.
*   Uncheck **allow file download**.

See [Disable ETP/FTP Server](/support/everything/etp#disable_etp_ftp_server) to remove the ETP server options and prevent the ETP server from starting.
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
    <code>etp\_client\_rewrite\_patterns=
    etp\_client\_rewrite\_substitutions=</code><br/>
    to:<br/>
    <code>etp\_client\_rewrite\_patterns=D:\\music;"D:\\\\Install Files"
    etp\_client\_rewrite\_substitutions=\\\\server\\music;"\\\\\\\\server\\\\Install Files"</code>
*   Save changes and restart Everything.

The pattern must match the path on the server. It is not effected by the link type.
<br/><br/><br/>

  

Running an ETP server as a service
----------------------------------

To run an Everything ETP server as a client service (not to be confused with the Everything service):

*   Copy your Everything.exe to an empty folder.
*   Run Everything.exe as administrator
*   Please make sure [Store settings and data in %APPDATA%\\Everything](/support/everything/options/#store_settings_and_data_in_appdata_everything) is disabled.
*   Please make sure the [Everything service](/support/everything/options/#everything_service) is not installed.
*   Setup your [indexes](/support/everything/options#indexes).
*   Setup the [ETP server settings](/support/everything/options/#etp_ftp_server).
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
*   [Everything 1.5 Plugins - ETP/FTP Server](https://www.voidtools.com/forum/viewtopic.php?p=35401#etp)
*   [Everything 1.5 Plugin SDK](https://www.voidtools.com/forum/viewtopic.php?t=16535)
