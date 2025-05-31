ETP/FTP Server Plugin for [Everything 1.5](https://www.voidtools.com/forum/viewtopic.php?f=12&t=9787)

Allow users to search and access your files from Everything or an FTP client.

[Download](#download)<br/>
[Install Guide](#Plug-in-Installation)<br/>
[Setup Guide](#Plug-in-Setup)<br/>
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



<h2 id="start_an_etp_ftp_server">Start an ETP/FTP server</h2>
<p>To start an ETP/FTP server:</p>
<ul>
<li><p>On the computer whose files you wish to share, In <b><i>"Everything"</i></b>, from the <b>Tools</b> menu, click <b>Options</b>.</p></li>
<li><p>Click the <b><i>ETP/FTP</i></b> tab.</p></li>
<li><p>Check <b>Enable ETP/FTP server</b>.</p></li>
<li><p>Click <b>OK</b>.</p></li>
</ul>
<br/>
<p>Please make sure you share each drive on your server as its drive letter.</p>
<p>For example, share C: drive as: <b>C</b></p>
<br/>
<p>To use custom shares, please see <a href="/support/everything/etp/#etp_client_path_rewriting">ETP path rewriting</a>.</p>
<br/>
<h2 id="connect_to_an_etp_server">Connect to an ETP server</h2>
<p>To connect to an ETP server:</p>
<ul>
<li><p>In <b>"Everything"</b>, from the <b>Tools</b> menu, click <b>connect to ETP server...</b>.</p></li>
<li><p>Type in the <b>server name</b> or <b>IP</b> for the host.</p></li>
<li><p>Click OK.</p></li>
</ul>
<br/>
<h2 id="etp_link_types">ETP link types</h2>
<p>There are four link types that change the way the Everything client accesses files on the ETP server.</p>
<p>When connecting to an ETP server you can specify the link type.</p>
<br/>
<table>
<tr><th class="wikinowrap">Link type</th><th class="wikinowrap">Description</th></tr>
<tr><td class="wikinowrap">C:</td><td>No change, the paths are the same as they are on the server. This is useful if you index a subst'ed drive on the server and are using mapped network drives on the client.</td></tr>
<tr><td class="wikinowrap">\\Server\C</td><td>Use Windows shares (this is the default link type). By default these shares do not exist, you will need to create them on the server if you wish to use this method.</td></tr>
<tr><td class="wikinowrap">\\Server\C$</td><td>Use the default admin drive shares.</td></tr>
<tr><td class="wikinowrap">ftp://host/C:</td><td>Use ftp links.</td></tr>
</table>
<br/>
<h2 id="username_and_password">Username and password</h2>
<p>To change the ETP/FTP server username and password:</p>
<ul>
<li><p>In <b><i>"Everything"</i></b>, from the <b>Tools</b> menu, click <b>Options</b>.</p></li>
<li><p>Click the <b>ETP/FTP Server</b> tab.</p></li>
<li><p>Type in a new <b>username</b> and <b>password</b>.</p></li>
<li><p>Click <b>OK</b>.</p></li>
</ul>
<br/>
<h2 id="disable_file_downloading">Disable file downloading</h2>
<p>To disable ETP/FTP file downloading:</p>
<ul>
<li><p>In <b><i>"Everything"</i></b>, from the <b>Tools</b> menu, click <b>Options</b>.</p></li>
<li><p>Click the <b>ETP/FTP Server</b> tab.</p></li>
<li><p>Uncheck <b>Allow file download</b>.</p></li>
<li><p>Click <b>OK</b></p></li>
</ul>
<br/>
<h2 id="different_indexes">Different indexes</h2>
<p>To index different NTFS volumes for the ETP server, see <a href="/support/everything/multiple_instances">Multiple Instances</a>.</p>
<br/>
<h2 id="create_a_windows_share">Create a Windows share</h2>
<p>Everything works best if you share each drive as a single letter, for example, share your C: drive as C, D: drive as D and so on.</p>
<br/>
<p>To create a windows share for a single folder, please follow the guide below:</p>
<br/>
<p>For example, we want to host an ETP/FTP server for one folder:</p>
<pre>C:\share</pre>
<br/>
<p>Substitute a drive letter for the local folder c:\share:</p>
<ul>
<li><p>From a command prompt, run:</p><pre>subst H: c:\share</pre>
</li>
</ul>
<br/>
<p>Setup an index to include only the H: drive.</p>
<ul>
<li><p>In <b><i>"Everything"</i></b>, from the <b>Tools</b> menu, click <b>Options</b>.</p></li>
<li><p>Click the <b>NTFS</b> tab.</p></li>
<li><p>Uncheck <b>Include in database</b> for all volumes, except the H: drive.</p></li>
<li><p>Click <b>OK</b>.</p></li>
</ul>
<br/>
<p>Add a share name to c:\share</p>
<ul>
<li><p>In <b>Explorer</b>, <b>right click</b> the <b>c:\share</b> and click <b>properties</b>.</p></li>
<li><p>Click the <b>Sharing</b> tab.</p></li>
<li><p>Click <b>Advanced Sharing...</b>.</p></li>
<li><p>Click <b>Add</b>.</p></li>
<li><p>Type in <b>H</b> for the <b>share name</b>.</p></li>
<li><p>Click <b>OK</b>.</p></li>
<li><p>Click <b>OK</b>.</p></li>
<li><p>Click <b>Close</b>.</p></li>
</ul>
<br/>
<p>Connect to the ETP/FTP server with \\Server\C links.</p>
<br/>
<h2 id="security">Security</h2>
<p>Every file and folder indexed by Everything can be searched and downloaded via the ETP server.</p>
<br/>
<p>To disable file downloading:</p>
<ul>
<li><p>In <b><i>Everything</i></b>, from the <b>Tools</b> menu, click <b>Options</b>.</p></li>
<li><p>Click the <b>ETP Server</b> tab.</p></li>
<li><p>Uncheck <b>allow file download</b>.</p></li>
</ul>
<br/>
<p>See <a href="/support/everything/etp#disable_etp_ftp_server">Disable ETP/FTP Server</a> to remove the ETP server options and prevent the ETP server from starting.</p>
<br/>
<h2 id="disable_etp_ftp_server">Disable ETP/FTP Server</h2>
<p>To disable the ETP/FTP server:</p>
<ul>
<li><p>Exit Everything (right click the Everything system tray icon and click Exit)</p></li>
<li><p>Open your Everything.ini in the same location as your Everything.exe</p></li>
<li><p>Change the following line:</p><pre>allow_etp_server=1</pre>
<p>to:</p>
<pre>allow_etp_server=0</pre>
</li>
<li><p>Save changes and restart Everything.</p></li>
</ul>
<br/>
<h2 id="etp_client_path_rewriting">ETP Client path rewriting</h2>
<p>ETP clients can rewrite the ETP server paths so they can be accessed from different shares.</p>
<p>For example, rewrite the path D:\music to \\server\music and "D:\Install Files" to "\\server\Install Files"</p>
<br/>
<p>To rewrite the paths on the ETP client.</p>
<ul>
<li><p>On the <b><i>Everything</i></b> ETP client PC:</p></li>
<li><p>Completely exit <b><i>Everything</i></b> (Right click the <b><i>Everything</i></b> system tray icon and click Exit)</p></li>
<li><p>Open your %APPDATA%\Everything\Everything.ini</p></li>
<li><p>Change the following lines:</p><p>etp_client_rewrite_patterns=</p>
<p>etp_client_rewrite_substitutions=</p>
<p>to:</p>
<p>etp_client_rewrite_patterns=D:\music;"D:\\Install Files"</p>
<p>etp_client_rewrite_substitutions=\\server\music;"\\\\server\\Install Files"</p>
</li>
<li><p>Save changes and restart Everything.</p></li>
</ul>
<br/>
<p>The pattern must match the path on the server. It is not effected by the link type.</p>
<br/>
<h2 id="running_an_etp_server_as_a_service">Running an ETP server as a service</h2>
<br/>
<p>To run an Everything ETP server as a client service (not to be confused with the Everything service):</p>
<ul>
<li><p>Copy your Everything.exe to an empty folder.</p></li>
<li><p>Run Everything.exe as administrator</p></li>
<li><p>Please make sure <a href="/support/everything/options/#store_settings_and_data_in_appdata_everything">Store settings and data in %APPDATA%\Everything</a> is disabled.</p></li>
<li><p>Please make sure the <a href="/support/everything/options/#everything_service">Everything service</a> is not installed.</p></li>
<li><p>Setup your <a href="/support/everything/options#indexes">indexes</a>.</p></li>
<li><p>Setup the <a href="/support/everything/options/#etp_ftp_server">ETP server settings</a>.</p></li>
<li><p>Completely exit Everything (right click the Everything tray icon and click Exit).</p></li>
<li><p>From a command prompt, navigate to your Everything.exe</p></li>
<li><p>Run the following command to install the client service:</p><pre>Everything.exe -install-client-service</pre>
<p>This will install and start the Everything ETP server as a service.</p>
</li>
</ul>
<br/>
<p>To uninstall the Everything client service:</p>
<ul>
<li><p>From a command prompt, navigate to your Everything.exe</p></li>
<li><p>Run the following command to install the client service:</p><pre>Everything.exe -uninstall-client-service</pre>
</li>
</ul>
<br/>
<h2 id="automatically_connect_to_an_etp_server">Automatically connect to an ETP server</h2>
<br/>
<p>To automatically connect to an ETP server when starting Everything:</p>
<ul>
<li><p>On the <b>"Everything"</b> client, from the <b>Tools</b> menu, click <b>Options</b>.</p></li>
<li><p>Click the <b>Home</b> tab.</p></li>
<li><p>Change <b>Index</b> to: <b>ETP server</b>.</p></li>
<li><p>Set the <b>ETP server</b> to: username:password@host:port</p></li>
<li><p>Set the desired <b>Link type</b>.</p></li>
<li><p>Click <b>OK</b>.</p></li>
</ul>
<br/>
<h2 id="trouble_shooting">Trouble Shooting</h2>
<p>Unable to start ETP server: bind failed 10048</p>
<p>Please make sure no FTP servers are already running on port 21 or use a different ETP server port.</p>
<br/>
<p>To change the ETP server port:</p>
<ul>
<li><p>In <b>"Everything"</b>, from the <b>Tools</b> menu, click <b>Options</b>.</p></li>
<li><p>Click the <b>ETP/FTP Server</b> tab.</p></li>
<li><p>Change the <b>port</b> to: <b>2121</b></p></li>
<li><p>Click <b>OK</b>.</p></li>
</ul>
<br/>
<p>Please match the same port number when connecting to the ETP server.</p>
<br/>



See also
--------

*   [Multiple Instances](https://www.voidtools.com/support/everything/multiple_instances)
*   https://www.voidtools.com/support/everything/etp/
*   [Everything 1.5 Plugins - ETP/FTP Server](https://www.voidtools.com/forum/viewtopic.php?p=35401#etp)
*   [Everything 1.5 Plugin SDK](https://www.voidtools.com/forum/viewtopic.php?t=16535)
