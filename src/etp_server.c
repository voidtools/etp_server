//
// Copyright 2025 voidtools / David Carpenter
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

/*
   5.1.  MINIMUM IMPLEMENTATION

      In order to make FTP workable without needless error messages, the
      following minimum implementation is required for all servers:

         TYPE - *ASCII Non-print
         MODE - *Stream
         STRUCTURE - *File, Record
         COMMANDS - *USER, *QUIT, *PORT,
                    *TYPE, *MODE, *STRU,
                      for the default values
                    *RETR, *STOR,
                    *NOOP.

      The default values for transfer parameters are:

         TYPE - ASCII Non-print
         MODE - Stream
         STRU - File

      All hosts must accept the above as the standard defaults.
      
      ------------------------------------------------------------------
      
		* Implemented
      
		Other commands implemented:
			
			SIZE
			MDTM
			LIST
			PASV
			CWD
			PWD
			XPWD
			OPTS
			FEAT
			SYST
			CDUP
			EPSV
			EPRT
			MLSD
			MLST
			EVERYTHING
			SITE
			
		Everything Commands
		
			QUERY (QUERY)
			EVERYTHING
				PATH	1|0
				CASE	1|0
				REGEX	1|0
				...
				QUERY	execute query
*/

#define _WIN32_IE 0x0501
#define _WIN32_WINNT 0x0501
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include "..\..\include\everything_plugin.h"
#include "version.h"

unsigned short __cdecl _byteswap_ushort(unsigned short _Short);

// client login states 
#define ETP_SERVER_CLIENT_LOGIN_STATE_NEED_USER						0
#define ETP_SERVER_CLIENT_LOGIN_STATE_INVALID_USER_NEED_PASSWORD	1
#define ETP_SERVER_CLIENT_LOGIN_STATE_NEED_PASSWORD					2	
#define ETP_SERVER_CLIENT_LOGIN_STATE_LOGGED_IN						3

#define ETP_SERVER_WM_LISTEN		(WM_USER)
#define ETP_SERVER_WM_CLIENT		(WM_USER+1)
#define ETP_SERVER_WM_PASV			(WM_USER+2)
#define ETP_SERVER_WM_DATA			(WM_USER+3)

#define ETP_SERVER_RECV_CHUNK_SIZE		65536
#define ETP_SERVER_LIST_CHUNK_SIZE		65536
#define ETP_SERVER_RETR_CHUNK_SIZE		65536

// data state.
#define ETP_SERVER_CLIENT_DATA_TYPE_NONE		0 // no data
#define ETP_SERVER_CLIENT_DATA_TYPE_LIST		1 // list
#define ETP_SERVER_CLIENT_DATA_TYPE_RETR		2 // retr
#define ETP_SERVER_CLIENT_DATA_TYPE_MLSD		3 // list

#define ETP_SERVER_CLIENT_DATA_CONNECTION_TYPE_NONE	0 
#define ETP_SERVER_CLIENT_DATA_CONNECTION_TYPE_PORT	1
#define ETP_SERVER_CLIENT_DATA_CONNECTION_TYPE_PASV	2
#define	ETP_SERVER_CLIENT_DATA_CONNECTION_TYPE_EPRT	3

#define ETP_SERVER_RECV_CHUNK_DATA(recv_chunk)		((everything_plugin_utf8_t *)(((etp_server_recv_chunk_t *)(recv_chunk)) + 1))

#define ETP_SERVER_DEFAULT_PORT			21
#define ETP_SERVER_DEFAULT_LOG_MAX_SIZE	(4 * 1024 * 1024)

#define ETP_SERVER_PLUGIN_API				WINAPI

// plugin ids
enum
{
	ETP_SERVER_PLUGIN_ID_ENABLED_CHECKBOX = 1000,
	ETP_SERVER_PLUGIN_ID_BINDINGS_STATIC,
	ETP_SERVER_PLUGIN_ID_BINDINGS_EDIT,
	ETP_SERVER_PLUGIN_ID_LOGGING_ENABLED_CHECKBOX,
	ETP_SERVER_PLUGIN_ID_ALLOW_FILE_DOWNLOAD_CHECKBOX,
	ETP_SERVER_PLUGIN_ID_PORT_STATIC,
	ETP_SERVER_PLUGIN_ID_PORT_EDIT,
	ETP_SERVER_PLUGIN_ID_USERNAME_STATIC,
	ETP_SERVER_PLUGIN_ID_USERNAME_EDIT,
	ETP_SERVER_PLUGIN_ID_PASSWORD_STATIC,
	ETP_SERVER_PLUGIN_ID_PASSWORD_EDIT,
	ETP_SERVER_PLUGIN_ID_LOG_FILE_NAME_EDIT,
	ETP_SERVER_PLUGIN_ID_LOG_FILE_NAME_BROWSE_BUTTON,
	ETP_SERVER_PLUGIN_ID_LOG_MAX_SIZE_EDIT,
	ETP_SERVER_PLUGIN_ID_LOG_FILE_STATIC,
	ETP_SERVER_PLUGIN_ID_MAX_SIZE_STATIC,
	ETP_SERVER_PLUGIN_ID_KB_STATIC,
	ETP_SERVER_PLUGIN_ID_RESTORE_DEFAULTS_BUTTON,
};

typedef struct etp_server_sort_name_to_id_s
{
	const everything_plugin_utf8_t *name;
	int property_type;
	char ascending;
	
}etp_server_sort_name_to_id_t;

// a 64k struct recv buffer chunk		
typedef struct etp_server_recv_chunk_s
{
	struct etp_server_recv_chunk_s *next;

#pragma pack (push,1)

	// data follows.	
	
}etp_server_recv_chunk_t;

#pragma pack (pop)

// a 64k struct recv buffer chunk		
typedef struct etp_server_send_packet_s
{
	struct etp_server_send_packet_s *next;
	
	uintptr_t size;

#pragma pack (push,1)

	// data follows.	
	
}etp_server_send_packet_t;

#pragma pack (pop)

// a 64k struct recv buffer chunk		
typedef struct etp_server_list_chunk_s
{
	struct etp_server_list_chunk_s *next;
	DWORD size;

#pragma pack (push,1)

	// data follows.
	
}etp_server_list_chunk_t;

#pragma pack (pop)

// client data
typedef struct etp_server_client_s
{
	EVERYTHING_PLUGIN_QWORD rest;
	EVERYTHING_PLUGIN_OS_WINSOCK_SOCKET control_socket;
	struct everything_plugin_os_winsock_sockaddr_storage control_addr;
	everything_plugin_db_query_t *db_query;
	int is_query;

	everything_plugin_utf8_t *last_search_string;
	int last_match_case;
	int last_match_whole_word;
	int last_match_path;
	int last_match_accents;

	int login_state;
	struct etp_server_client_s *next;
	struct etp_server_client_s *prev;
	int is_ipv6;

	everything_plugin_utf8_t *working_directory;

	int utf8_on;
	int is_quit;
	
	// query state.
	int match_case;
	int match_whole_word;
	int match_path;
	int match_diacritics;
	int match_prefix;
	int match_suffix;
	int ignore_punctuation;
	int ignore_whitespace;
	int match_regex;
	int hide_empty_search_results;
	everything_plugin_utf8_t *search_string;
	everything_plugin_utf8_t *filter_search;
	DWORD filter_flags;
	const everything_plugin_property_t *sort_property_type;
	int sort_ascending;
	DWORD offset;
	DWORD count;
	int size_column;
	int attributes_column;
	int date_modified_column;
	int date_created_column;
	int path_column;
	int file_list_filename_column;
	int date_recently_changed_column;
	
	// current query
	int current_match_case;
	int current_match_whole_word;
	int current_match_path;
	int current_match_diacritics;
	int current_match_prefix;
	int current_match_suffix;
	int current_ignore_punctuation;
	int current_ignore_whitespace;
	int current_match_regex;
	int current_hide_empty_search_results;
	everything_plugin_utf8_t *current_search_string;
	everything_plugin_utf8_t *current_filter_search;
	DWORD current_filter_search_flags;
	const everything_plugin_property_t *current_sort_column_type;
	int current_sort_ascending;
	int is_current_query;
	int current_size_column;
	int current_attributes_column;
	int current_date_modified_column;
	int current_date_created_column;
	int current_path_column;
	int current_file_list_filename_column;
	int current_date_recently_changed_column;
	
	// recv packet buffers
	etp_server_recv_chunk_t *recv_chunk_start;
	etp_server_recv_chunk_t *recv_chunk_last;
	char *recv_front;
	char *recv_end;
	uintptr_t recv_chunk_count;
	
	// send packets
	etp_server_send_packet_t *send_start;
	etp_server_send_packet_t *send_last;
	uintptr_t send_remaining;
	
	// data params
	int data_connection_type;
	int data_is_connected;
	
	// connection data, depending on data_connection_type.
	union
	{
		struct
		{
			BYTE ip[4];
			int port;
		}port;
		
		struct everything_plugin_os_winsock_sockaddr_storage eprt_addr;
		
		struct
		{
			EVERYTHING_PLUGIN_OS_WINSOCK_SOCKET socket_handle;
		}pasv;
		
	}data_connection_data;

	int data_type;
	EVERYTHING_PLUGIN_OS_WINSOCK_SOCKET data_socket;
	int data_complete;
	
	// data, depending on data_type.
	union
	{
		// list data
		struct
		{
			etp_server_list_chunk_t *chunk_start;
			etp_server_list_chunk_t *chunk_last;
			uintptr_t remaining;
			everything_plugin_utf8_t *path;
			
		}list;
		
		// list data
		struct
		{
			etp_server_list_chunk_t *chunk_start;
			etp_server_list_chunk_t *chunk_last;
			uintptr_t remaining;
			
		}everything_query;
				
		// retr data
		struct
		{
			// MUST be accessed inside CS
			char *buffer;
			uintptr_t size; // number of bytes saved into buffer.
			HANDLE file;
			uintptr_t remaining;
			HANDLE thread;  // NULL if we got EOF.
			HANDLE hevent;
			CRITICAL_SECTION cs;
			int abort;
			int state; // 0 = ok, 1 = EOF, 2 read error.
			
		}retr;		
		
	}data;
	
}etp_server_client_t;

// types
typedef struct etp_server_listen_s
{
	struct etp_server_listen_s *next;
	EVERYTHING_PLUGIN_OS_WINSOCK_SOCKET listen_socket;

}etp_server_listen_t;

// server data
typedef struct etp_server_s
{
	etp_server_listen_t *listen_start;
	etp_server_listen_t *listen_last;

	etp_server_client_t *client_start;
	etp_server_client_t *client_last;
	HWND hwnd;
	
	// ref to db.
	everything_plugin_db_t *db;
	
	int port;
	everything_plugin_utf8_t *bindings;
	
	everything_plugin_output_stream_t *log_file;
	
}etp_server_t;

typedef struct etp_server_everything_plugin_proc_s
{
	const everything_plugin_utf8_t *name;
	void **proc_address_ptr;
	
}etp_server_everything_plugin_proc_t;

// client funcs
static etp_server_client_t *etp_server_client_create(EVERYTHING_PLUGIN_OS_WINSOCK_SOCKET socket_handle);
static void etp_server_client_destroy(etp_server_client_t *c);
static etp_server_client_t *etp_server_client_find_control_socket(EVERYTHING_PLUGIN_OS_WINSOCK_SOCKET socket_handle);
static etp_server_client_t *etp_server_client_find_pasv_socket(EVERYTHING_PLUGIN_OS_WINSOCK_SOCKET socket_handle);
static etp_server_client_t *etp_server_client_find_data_socket(EVERYTHING_PLUGIN_OS_WINSOCK_SOCKET socket_handle);
static int etp_server_client_update_recv(etp_server_client_t *c);
static int etp_server_client_update_send(etp_server_client_t *c);
static int etp_server_client_update_data(etp_server_client_t *c);
static void etp_server_client_process_command(etp_server_client_t *c,everything_plugin_utf8_t *command);
static void etp_server_client_printf(etp_server_client_t *c,const everything_plugin_utf8_t *format,...);
static void etp_server_client_EPSV(etp_server_client_t *c);
static void etp_server_client_EPRT(etp_server_client_t *c,const everything_plugin_utf8_t *address);
static void etp_server_client_PORT(etp_server_client_t *c,const everything_plugin_utf8_t *address);
static void etp_server_client_LIST(etp_server_client_t *c,const everything_plugin_utf8_t *ftp_pathname);
static void etp_server_client_MLSD(etp_server_client_t *c,const everything_plugin_utf8_t *ftp_pathname);
static void etp_server_client_print_MLST(etp_server_client_t *c,int is_folder,EVERYTHING_PLUGIN_QWORD modify,EVERYTHING_PLUGIN_QWORD size,const everything_plugin_utf8_t *full_ftp_filename);
static void etp_server_client_MLST(etp_server_client_t *c,const everything_plugin_utf8_t *ftp_filename);
static void etp_server_get_ftp_filename(etp_server_client_t *c,everything_plugin_utf8_buf_t *cbuf,const everything_plugin_utf8_t *filename);
static void etp_server_client_everything(etp_server_client_t *c,const everything_plugin_utf8_t *command,const everything_plugin_utf8_t *param);
static void etp_server_client_CWD(etp_server_client_t *c,const everything_plugin_utf8_t *ftp_path);
static void etp_server_client_A(etp_server_client_t *c,const everything_plugin_utf8_t *format,...);
static void etp_server_client_UTF8(etp_server_client_t *c,const everything_plugin_utf8_t *format,...);
static void etp_server_list_chunk_add(etp_server_client_t *c,const void *data,uintptr_t size);
static void etp_server_client_CDUP(etp_server_client_t *c);
static void etp_server_client_MDTM(etp_server_client_t *c,const everything_plugin_utf8_t *ftp_filename);
static void etp_server_SIZE(etp_server_client_t *c,const everything_plugin_utf8_t *ftp_filename);
static void etp_server_log(etp_server_client_t *c,const everything_plugin_utf8_t *format,...);
static void EVERYTHING_PLUGIN_API etp_server_db_query_event_proc(etp_server_client_t *c,int type);
static LRESULT WINAPI etp_server_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
static int etp_server_get_bind_addrinfo(const everything_plugin_utf8_t *nodename,struct everything_plugin_os_winsock_addrinfo **ai);
static void etp_server_get_full_filename(etp_server_client_t *c,everything_plugin_utf8_buf_t *cbuf,const everything_plugin_utf8_t *ftp_filename);
static everything_plugin_utf8_t *etp_server_get_welcome_message(everything_plugin_utf8_t *buf,const everything_plugin_utf8_t *message);
static void etp_server_recv_chunk_add(etp_server_client_t *c);
static void etp_server_recv_chunk_destroy(etp_server_recv_chunk_t *recv_chunk);
static void etp_server_send_packet_add(etp_server_client_t *c,void *data,uintptr_t size);
static void etp_server_send_packet_destroy(etp_server_send_packet_t *send_packet);
static void etp_server_RETR(etp_server_client_t *c,const everything_plugin_utf8_t *ftp_filename);
static void etp_server_close_data(etp_server_client_t *c);
static void etp_server_PASV(etp_server_client_t *c);
static DWORD WINAPI etp_server_retr_thread_proc(etp_server_client_t *c);
static void etp_server_open_data(etp_server_client_t *c);
static void etp_server_run_data_command(etp_server_client_t *c);
static int etp_server_is_config_change(void);
static void etp_server_send_query_results(etp_server_client_t *c);
static const everything_plugin_property_t *etp_server_get_sort_property_from_name(const everything_plugin_utf8_t *sort_name);
static int etp_server_get_sort_ascending_from_name(const everything_plugin_utf8_t *sort_name);
static int etp_server_add_binding(everything_plugin_utf8_buf_t *error_cbuf,const everything_plugin_utf8_t *nodename);
static int etp_server_validate_ftp_data_connection_ip(EVERYTHING_PLUGIN_OS_WINSOCK_SOCKET socket_handle,struct everything_plugin_os_winsock_sockaddr *addr);
static int etp_server_addr_is_host_equal(struct everything_plugin_os_winsock_sockaddr *a,struct everything_plugin_os_winsock_sockaddr *b);
static void etp_server_update_options_page(HWND page_hwnd);
static void etp_server_shutdown(void);
static int etp_server_apply_settings(void);
static void etp_server_start(void);
static void etp_server_create_checkbox(everything_plugin_load_options_page_t *load_options_page,int id,DWORD extra_style,int text_localization_id,int tooltip_localization_id,int checked);
static void etp_server_create_static(everything_plugin_load_options_page_t *load_options_page,int id,int text_localization_id);
static void etp_server_create_edit(everything_plugin_load_options_page_t *load_options_page,int id,int tooltip_localization_id,const everything_plugin_utf8_t *text);
static void etp_server_create_number_edit(everything_plugin_load_options_page_t *load_options_page,int id,int tooltip_localization_id,int number);
static void etp_server_create_password_edit(everything_plugin_load_options_page_t *load_options_page,int id,int tooltip_localization_id,const everything_plugin_utf8_t *text);
static void etp_server_create_button(everything_plugin_load_options_page_t *load_options_page,int id,DWORD extra_style,int text_localization_id,int tooltip_localization_id);
static void etp_server_enable_options_apply(everything_plugin_options_page_proc_t *options_page_proc);
static int etp_server_expand_min_wide(HWND page_hwnd,int text_localization_id,int current_wide);
static everything_plugin_utf8_t *etp_server_get_options_text(HWND page_hwnd,int id,everything_plugin_utf8_t *old_value);

// static vars
static etp_server_t *_etp_server = 0;

// ETP server
static int etp_server_enabled = 0;
static everything_plugin_utf8_t *etp_server_welcome_message = 0;
static int etp_server_port = ETP_SERVER_DEFAULT_PORT;
static everything_plugin_utf8_t *etp_server_username = 0;
static everything_plugin_utf8_t *etp_server_password = 0;
static everything_plugin_utf8_t *etp_server_log_file_name = 0;
static int etp_server_logging_enabled = 1;
static int etp_server_log_max_size = ETP_SERVER_DEFAULT_LOG_MAX_SIZE;
static int etp_server_log_delta_size = 512 * 1024;
static int etp_server_allow_file_download = 1;
static everything_plugin_utf8_t *etp_server_bindings = 0;
static int etp_server_allow_ftp_port = 1;
static int etp_server_check_ftp_data_connection_ip = 1;
static int etp_server_allow_disk_access = 0;

// there is no Type Name because that should be done client side.
static etp_server_sort_name_to_id_t etp_server_sort_name_to_ids[] =
{
	{(const everything_plugin_utf8_t *)"name_ascending",EVERYTHING_PLUGIN_PROPERTY_TYPE_NAME,1},
	{(const everything_plugin_utf8_t *)"name_descending",EVERYTHING_PLUGIN_PROPERTY_TYPE_NAME,0},
	{(const everything_plugin_utf8_t *)"path_ascending",EVERYTHING_PLUGIN_PROPERTY_TYPE_PATH,1},
	{(const everything_plugin_utf8_t *)"path_descending",EVERYTHING_PLUGIN_PROPERTY_TYPE_PATH,0},
	{(const everything_plugin_utf8_t *)"size_ascending",EVERYTHING_PLUGIN_PROPERTY_TYPE_SIZE,1},
	{(const everything_plugin_utf8_t *)"size_descending",EVERYTHING_PLUGIN_PROPERTY_TYPE_SIZE,0},
	{(const everything_plugin_utf8_t *)"inverse_size_ascending",EVERYTHING_PLUGIN_PROPERTY_TYPE_SIZE,0},
	{(const everything_plugin_utf8_t *)"inverse_size_descending",EVERYTHING_PLUGIN_PROPERTY_TYPE_SIZE,1},
	{(const everything_plugin_utf8_t *)"extension_ascending",EVERYTHING_PLUGIN_PROPERTY_TYPE_EXTENSION,1},
	{(const everything_plugin_utf8_t *)"extension_descending",EVERYTHING_PLUGIN_PROPERTY_TYPE_EXTENSION,0},
	{(const everything_plugin_utf8_t *)"date_created_ascending",EVERYTHING_PLUGIN_PROPERTY_TYPE_DATE_CREATED,1},
	{(const everything_plugin_utf8_t *)"date_created_descending",EVERYTHING_PLUGIN_PROPERTY_TYPE_DATE_CREATED,0},
	{(const everything_plugin_utf8_t *)"date_modified_ascending",EVERYTHING_PLUGIN_PROPERTY_TYPE_DATE_MODIFIED,1},
	{(const everything_plugin_utf8_t *)"date_modified_descending",EVERYTHING_PLUGIN_PROPERTY_TYPE_DATE_MODIFIED,0},
	{(const everything_plugin_utf8_t *)"attributes_ascending",EVERYTHING_PLUGIN_PROPERTY_TYPE_ATTRIBUTES,1},
	{(const everything_plugin_utf8_t *)"attributes_descending",EVERYTHING_PLUGIN_PROPERTY_TYPE_ATTRIBUTES,0},
	{(const everything_plugin_utf8_t *)"file_list_filename_ascending",EVERYTHING_PLUGIN_PROPERTY_TYPE_FILE_LIST_FILENAME,1},
	{(const everything_plugin_utf8_t *)"file_list_filename_descending",EVERYTHING_PLUGIN_PROPERTY_TYPE_FILE_LIST_FILENAME,0},
	{(const everything_plugin_utf8_t *)"date_recently_changed_ascending",EVERYTHING_PLUGIN_PROPERTY_TYPE_DATE_RECENTLY_CHANGED,1},
	{(const everything_plugin_utf8_t *)"date_recently_changed_descending",EVERYTHING_PLUGIN_PROPERTY_TYPE_DATE_RECENTLY_CHANGED,0},
};

#define ETP_SERVER_NUM_SORT_NAME_TO_IDS	(sizeof(etp_server_sort_name_to_ids) / sizeof(etp_server_sort_name_to_id_t))

static void *(EVERYTHING_PLUGIN_API *everything_plugin_mem_alloc)(uintptr_t size);
static void *(EVERYTHING_PLUGIN_API *everything_plugin_mem_calloc)(uintptr_t size);
static void (EVERYTHING_PLUGIN_API *everything_plugin_mem_free)(void *ptr);
static void (EVERYTHING_PLUGIN_API *everything_plugin_os_thread_wait_and_close)(everything_plugin_os_thread_t *t);
static int (EVERYTHING_PLUGIN_API *everything_plugin_os_winsock_closesocket)(EVERYTHING_PLUGIN_OS_WINSOCK_SOCKET s);
static void (EVERYTHING_PLUGIN_API *everything_plugin_utf8_buf_init)(everything_plugin_utf8_buf_t *cbuf);
static void (EVERYTHING_PLUGIN_API *everything_plugin_utf8_buf_kill)(everything_plugin_utf8_buf_t *cbuf);
static const everything_plugin_utf8_t *(EVERYTHING_PLUGIN_API *everything_plugin_localization_get_string)(int id);
static void (EVERYTHING_PLUGIN_API *everything_plugin_utf8_buf_path_cat_filename)(everything_plugin_utf8_buf_t *cbuf,const everything_plugin_utf8_t *path,const everything_plugin_utf8_t *filename);
static everything_plugin_utf8_t *(EVERYTHING_PLUGIN_API *everything_plugin_get_setting_string)(struct sorted_list_s *sorted_list,const everything_plugin_utf8_t *name,everything_plugin_utf8_t *current_string);
static int (EVERYTHING_PLUGIN_API *everything_plugin_get_setting_int)(struct sorted_list_s *sorted_list,const everything_plugin_utf8_t *name,int current_value);
static void (EVERYTHING_PLUGIN_API *everything_plugin_debug_printf)(const everything_plugin_utf8_t *format,...);
static void (EVERYTHING_PLUGIN_API *everything_plugin_utf8_buf_printf)(everything_plugin_utf8_buf_t *cbuf,const everything_plugin_utf8_t *format,...);
static int (EVERYTHING_PLUGIN_API *everything_plugin_utf8_string_compare_nocase_s_sla)(const everything_plugin_utf8_t *s1start,const everything_plugin_utf8_t *lowercase_ascii_s2start);
static HANDLE (EVERYTHING_PLUGIN_API *everything_plugin_os_open_file)(const everything_plugin_utf8_t *filename);
static void (EVERYTHING_PLUGIN_API *everything_plugin_utf8_buf_copy_utf8_string)(everything_plugin_utf8_buf_t *cbuf,const everything_plugin_utf8_t *s);
static void (EVERYTHING_PLUGIN_API *everything_plugin_utf8_buf_copy_utf8_string_n)(everything_plugin_utf8_buf_t *cbuf,const everything_plugin_utf8_t *s,uintptr_t slen);
static HANDLE (EVERYTHING_PLUGIN_API *everything_plugin_os_event_create)(void);
static everything_plugin_os_thread_t *(EVERYTHING_PLUGIN_API *everything_plugin_os_thread_create)(DWORD (WINAPI *thread_proc)(void *),void *param);
static int (EVERYTHING_PLUGIN_API *everything_plugin_db_folder_exists)(everything_plugin_db_t *db,const everything_plugin_utf8_t *filename);
static int (EVERYTHING_PLUGIN_API *everything_plugin_db_file_exists)(everything_plugin_db_t *db,const everything_plugin_utf8_t *filename);
static everything_plugin_db_find_t *(EVERYTHING_PLUGIN_API *everything_plugin_db_find_first_file)(everything_plugin_db_t *db,const everything_plugin_utf8_t *path,everything_plugin_utf8_buf_t *filename_cbuf,everything_plugin_fileinfo_fd_t *fd);
static int (EVERYTHING_PLUGIN_API *everything_plugin_db_find_next_file)(everything_plugin_db_find_t *fh,everything_plugin_utf8_buf_t *filename_cbuf,everything_plugin_fileinfo_fd_t *fd);
static void (EVERYTHING_PLUGIN_API *everything_plugin_db_find_close)(everything_plugin_db_find_t *fh);
static uintptr_t (EVERYTHING_PLUGIN_API *everything_plugin_safe_uintptr_add)(uintptr_t a,uintptr_t b);
static void (EVERYTHING_PLUGIN_API *everything_plugin_os_copy_memory)(void *dst,const void *src,uintptr_t size);
static int (EVERYTHING_PLUGIN_API *everything_plugin_debug_is_verbose)(void);
static everything_plugin_utf8_t *(EVERYTHING_PLUGIN_API *everything_plugin_utf8_string_realloc_utf8_string)(everything_plugin_utf8_t *ptr,const everything_plugin_utf8_t *s);
static int (EVERYTHING_PLUGIN_API *everything_plugin_utf8_string_compare)(const everything_plugin_utf8_t *start1,const everything_plugin_utf8_t *start2);
static DWORD (EVERYTHING_PLUGIN_API *everything_plugin_utf8_string_to_dword)(const everything_plugin_utf8_t *s);
static void (EVERYTHING_PLUGIN_API *everything_plugin_utf8_buf_path_canonicalize)(everything_plugin_utf8_buf_t *cbuf);
static void (EVERYTHING_PLUGIN_API *everything_plugin_utf8_buf_vprintf)(everything_plugin_utf8_buf_t *cbuf,const everything_plugin_utf8_t *format,va_list argptr);
static void (EVERYTHING_PLUGIN_API *everything_plugin_debug_color_printf)(DWORD color,const everything_plugin_utf8_t *format,...);
static void (EVERYTHING_PLUGIN_API *everything_plugin_os_get_local_app_data_path_cat_make_filename)(const everything_plugin_utf8_t *name,const everything_plugin_utf8_t *extension,everything_plugin_utf8_buf_t *cbuf);
static int (EVERYTHING_PLUGIN_API *everything_plugin_os_get_local_app_data_path_cat_filename)(const everything_plugin_utf8_t *filename,everything_plugin_utf8_buf_t *cbuf);
static void (EVERYTHING_PLUGIN_API *everything_plugin_os_resize_file)(const everything_plugin_utf8_t *filename,uintptr_t max_size,uintptr_t delta_size);
static void (EVERYTHING_PLUGIN_API *everything_plugin_os_make_sure_path_to_file_exists)(const everything_plugin_utf8_t *file_name);
static everything_plugin_output_stream_t *(EVERYTHING_PLUGIN_API *everything_plugin_output_stream_append_file)(const everything_plugin_utf8_t *filename);
static void (EVERYTHING_PLUGIN_API *everything_plugin_output_stream_close)(everything_plugin_output_stream_t *s);
static EVERYTHING_PLUGIN_QWORD (EVERYTHING_PLUGIN_API *everything_plugin_os_get_system_time_as_file_time)(void);
static void (EVERYTHING_PLUGIN_API *everything_plugin_version_get_text)(everything_plugin_utf8_buf_t *cbuf);
static void (EVERYTHING_PLUGIN_API *everything_plugin_utf8_buf_format_filetime)(everything_plugin_utf8_buf_t *cbuf,EVERYTHING_PLUGIN_QWORD ft);
static void (EVERYTHING_PLUGIN_API *everything_plugin_output_stream_write_printf)(everything_plugin_output_stream_t *output_stream,const everything_plugin_utf8_t *format,...);
static void (EVERYTHING_PLUGIN_API *everything_plugin_utf8_buf_format_peername)(everything_plugin_utf8_buf_t *cbuf,EVERYTHING_PLUGIN_OS_WINSOCK_SOCKET socket_handle);
static void (EVERYTHING_PLUGIN_API *everything_plugin_utf8_buf_grow_length)(everything_plugin_utf8_buf_t *cbuf,uintptr_t length_in_bytes);
static int (EVERYTHING_PLUGIN_API *everything_plugin_os_filetime_to_localtime)(SYSTEMTIME *localst,EVERYTHING_PLUGIN_QWORD ft);
static void (EVERYTHING_PLUGIN_API *everything_plugin_os_zero_memory)(void *ptr,uintptr_t size);
static int (EVERYTHING_PLUGIN_API *everything_plugin_os_winsock_getaddrinfo)(const char *nodename,const char *servname,const struct everything_plugin_os_winsock_addrinfo* hints,struct everything_plugin_os_winsock_addrinfo** res);
static EVERYTHING_PLUGIN_OS_WINSOCK_SOCKET (EVERYTHING_PLUGIN_API *everything_plugin_os_winsock_socket)(int af,int type,int protocol);
static int (EVERYTHING_PLUGIN_API *everything_plugin_os_winsock_bind)(EVERYTHING_PLUGIN_OS_WINSOCK_SOCKET s,const struct everything_plugin_os_winsock_sockaddr *name,int namelen);
static int (EVERYTHING_PLUGIN_API *everything_plugin_os_winsock_listen)(EVERYTHING_PLUGIN_OS_WINSOCK_SOCKET s,int backlog);
static int (EVERYTHING_PLUGIN_API *everything_plugin_os_winsock_WSAAsyncSelect)(EVERYTHING_PLUGIN_OS_WINSOCK_SOCKET s,HWND hWnd,unsigned int wMsg,long lEvent);
static int (EVERYTHING_PLUGIN_API *everything_plugin_os_winsock_WSAGetLastError)(void);
static void (EVERYTHING_PLUGIN_API *everything_plugin_os_winsock_freeaddrinfo)(struct everything_plugin_os_winsock_addrinfo* ai);
static int (EVERYTHING_PLUGIN_API *everything_plugin_os_winsock_WSAStartup)(WORD wVersionRequested,EVERYTHING_PLUGIN_OS_WINSOCK_WSADATA *lpWSAData);
static everything_plugin_utf8_t *(EVERYTHING_PLUGIN_API *everything_plugin_utf8_string_alloc_utf8_string)(const everything_plugin_utf8_t *s);
static everything_plugin_db_t *(EVERYTHING_PLUGIN_API *everything_plugin_db_add_local_ref)(void);
static void (EVERYTHING_PLUGIN_API *everything_plugin_db_release)(everything_plugin_db_t *db);
static everything_plugin_db_query_t *(EVERYTHING_PLUGIN_API *everything_plugin_db_query_create)(everything_plugin_db_t *db,void (EVERYTHING_PLUGIN_API *event_proc)(void *user_data,int type),void *user_data);
static void (EVERYTHING_PLUGIN_API *everything_plugin_db_query_destroy)(everything_plugin_db_query_t *q);
static void (EVERYTHING_PLUGIN_API *everything_plugin_os_register_class)(UINT style,const everything_plugin_utf8_t *lpszClassName,WNDPROC lpfnWndProc,uintptr_t window_extra,HICON hIcon,HICON hIconSm,HCURSOR hcursor);
static HWND (EVERYTHING_PLUGIN_API *everything_plugin_os_create_window)(DWORD dwExStyle,const everything_plugin_utf8_t *lpClassName,const everything_plugin_utf8_t *lpWindowName,DWORD dwStyle,int x,int y,int nWidth,int nHeight,HWND hWndParent,HMENU hMenu,HINSTANCE hInstance,LPVOID lpParam);
static const everything_plugin_utf8_t *(EVERYTHING_PLUGIN_API *everything_plugin_utf8_string_parse_csv_item)(const everything_plugin_utf8_t *s,everything_plugin_utf8_buf_t *cbuf);
static int (EVERYTHING_PLUGIN_API *everything_plugin_ui_task_dialog_show)(HWND parent_hwnd,UINT flags,const everything_plugin_utf8_t *caption,const everything_plugin_utf8_t *main_task,const everything_plugin_utf8_t *format,...);
static int (EVERYTHING_PLUGIN_API *everything_plugin_os_winsock_WSACleanup)(void);
static uintptr_t (EVERYTHING_PLUGIN_API *everything_plugin_db_query_get_result_count)(const everything_plugin_db_query_t *q);
static void (EVERYTHING_PLUGIN_API *everything_plugin_db_query_get_result_name)(everything_plugin_db_query_t *q,uintptr_t index,everything_plugin_utf8_buf_t *cbuf);
static void (EVERYTHING_PLUGIN_API *everything_plugin_db_query_get_result_path)(everything_plugin_db_query_t *q,uintptr_t index,everything_plugin_utf8_buf_t *cbuf);
static void (EVERYTHING_PLUGIN_API *everything_plugin_db_query_get_result_indexed_fd)(everything_plugin_db_query_t *q,uintptr_t index,everything_plugin_fileinfo_fd_t *fd);
static int (EVERYTHING_PLUGIN_API *everything_plugin_db_query_is_folder_result)(everything_plugin_db_query_t *q,uintptr_t index);
static EVERYTHING_PLUGIN_OS_WINSOCK_SOCKET (EVERYTHING_PLUGIN_API *everything_plugin_os_winsock_accept)(EVERYTHING_PLUGIN_OS_WINSOCK_SOCKET s,struct everything_plugin_os_sockaddr *addr,int *addrlen);
static void (EVERYTHING_PLUGIN_API *everything_plugin_network_set_tcp_nodelay)(EVERYTHING_PLUGIN_OS_WINSOCK_SOCKET socket_handle);
static void (EVERYTHING_PLUGIN_API *everything_plugin_network_set_keepalive)(EVERYTHING_PLUGIN_OS_WINSOCK_SOCKET socket_handle);
static int (EVERYTHING_PLUGIN_API *everything_plugin_network_recv)(EVERYTHING_PLUGIN_OS_WINSOCK_SOCKET s,void *buf,uintptr_t len);
static int (EVERYTHING_PLUGIN_API *everything_plugin_network_send)(EVERYTHING_PLUGIN_OS_WINSOCK_SOCKET s,const void *data,uintptr_t len);
static void (EVERYTHING_PLUGIN_API *everything_plugin_os_move_memory)(void *dst,const void *src,uintptr_t size);
static int (EVERYTHING_PLUGIN_API *everything_plugin_os_winsock_shutdown)(EVERYTHING_PLUGIN_OS_WINSOCK_SOCKET s,int how);
static uintptr_t (EVERYTHING_PLUGIN_API *everything_plugin_utf8_string_get_length_in_bytes)(const everything_plugin_utf8_t *string);
static const everything_plugin_property_t *(EVERYTHING_PLUGIN_API *everything_plugin_property_get_builtin_type)(int type);
static void (EVERYTHING_PLUGIN_API *everything_plugin_db_query_sort)(everything_plugin_db_query_t *q,const everything_plugin_property_t *column_type,int ascending,const everything_plugin_property_t *column_type2,int ascending2,const everything_plugin_property_t *column_type3,int ascending3,int folders_first,int force,int find_duplicate_type,int sort_mix);
static void (EVERYTHING_PLUGIN_API *everything_plugin_db_query_search2)(everything_plugin_db_query_t *q,int match_case,int match_whole_word,int match_path,int match_diacritics,int match_prefix,int match_suffix,int ignore_punctuation,int ignore_whitespace,int match_regex,int hide_empty_search_results,int clear_selection,int clear_item_refs,const everything_plugin_utf8_t *search_string,DWORD filter_flags,const everything_plugin_utf8_t *filter,const everything_plugin_utf8_t *filter_columns,const everything_plugin_property_t *filter_sort,int filter_sort_ascending,int filter_view,int fast_sort_only,const everything_plugin_property_t *sort_property_type,int sort_ascending,const everything_plugin_property_t *sort_property_type2,int sort_ascending2,const everything_plugin_property_t *sort_property_type3,int sort_ascending3,int folders_first,int dialog_center_x,int dialog_center_y,int track_selected_and_total_file_size,int track_selected_folder_size,int force,int allow_query_access,int allow_read_access,int allow_disk_access,int hide_omit_results,int size_standard,int match_treeview,int treeview_subfolders,int sort_mix);
static struct everything_plugin_ui_options_page_s *(EVERYTHING_PLUGIN_API *everything_plugin_ui_options_add_plugin_page)(struct everything_plugin_ui_options_add_custom_page_s *add_custom_page,void *user_data,const everything_plugin_utf8_t *name);
static void (EVERYTHING_PLUGIN_API *everything_plugin_set_setting_int)(struct everything_plugin_output_stream_s *output_stream,const everything_plugin_utf8_t *name,int value);
static void (EVERYTHING_PLUGIN_API *everything_plugin_set_setting_string)(everything_plugin_output_stream_t *output_stream,const everything_plugin_utf8_t *name,const everything_plugin_utf8_t *value);
static int (EVERYTHING_PLUGIN_API *everything_plugin_os_get_logical_wide)(void);
static int (EVERYTHING_PLUGIN_API *everything_plugin_os_get_logical_high)(void);
static void (EVERYTHING_PLUGIN_API *everything_plugin_os_set_dlg_rect)(HWND parent_hwnd,int id,int x,int y,int wide,int high);
static int (EVERYTHING_PLUGIN_API *everything_plugin_os_set_dlg_text)(HWND hDlg,int nIDDlgItem,const everything_plugin_utf8_t *s);
static void (EVERYTHING_PLUGIN_API *everything_plugin_os_get_dlg_text)(HWND hwnd,int id,everything_plugin_utf8_buf_t *cbuf);
static void (EVERYTHING_PLUGIN_API *everything_plugin_os_enable_or_disable_dlg_item)(HWND parent_hwnd,int id,int enable);
static int (EVERYTHING_PLUGIN_API *everything_plugin_os_get_save_file_name)(HWND parent,const everything_plugin_utf8_t *title,const everything_plugin_utf8_t *initial_file,everything_plugin_utf8_t *filter,uintptr_t filter_len,DWORD filter_index,const everything_plugin_utf8_t *default_extension,DWORD *out_filter_index,everything_plugin_utf8_buf_t *cbuf);
static HWND (EVERYTHING_PLUGIN_API *everything_plugin_os_create_checkbox)(HWND parent,int id,DWORD extra_style,int checked,const everything_plugin_utf8_t *text);
static void (EVERYTHING_PLUGIN_API *everything_plugin_os_add_tooltip)(HWND tooltip,HWND parent,int id,const everything_plugin_utf8_t *text);
static HWND (EVERYTHING_PLUGIN_API *everything_plugin_os_create_static)(HWND parent,int id,DWORD extra_window_style,const everything_plugin_utf8_t *text);
static HWND (EVERYTHING_PLUGIN_API *everything_plugin_os_create_edit)(HWND parent,int id,DWORD extra_style,const everything_plugin_utf8_t *text);
static HWND (EVERYTHING_PLUGIN_API *everything_plugin_os_create_number_edit)(HWND parent,int id,DWORD extra_style,__int64 number);
static HWND (EVERYTHING_PLUGIN_API *everything_plugin_os_create_password_edit)(HWND parent,int id,DWORD extra_style,const everything_plugin_utf8_t *text);
static HWND (EVERYTHING_PLUGIN_API *everything_plugin_os_create_button)(HWND parent,int id,DWORD extra_window_style,const everything_plugin_utf8_t *text);
static int (EVERYTHING_PLUGIN_API *everything_plugin_os_expand_dialog_text_logical_wide_no_prefix)(HWND parent,const everything_plugin_utf8_t *text,int wide);
static int (EVERYTHING_PLUGIN_API *everything_plugin_os_winsock_getpeername)(EVERYTHING_PLUGIN_OS_WINSOCK_SOCKET s,struct everything_plugin_os_winsock_sockaddr *name,int * namelen);
static int (EVERYTHING_PLUGIN_API *everything_plugin_os_winsock_getsockname)(EVERYTHING_PLUGIN_OS_WINSOCK_SOCKET s,struct everything_plugin_os_winsock_sockaddr *name,int *namelen);
static int (EVERYTHING_PLUGIN_API *everything_plugin_unicode_is_ascii_ws)(int c);
static EVERYTHING_PLUGIN_QWORD (EVERYTHING_PLUGIN_API *everything_plugin_utf8_string_to_qword)(const everything_plugin_utf8_t *s);
static void (EVERYTHING_PLUGIN_API *everything_plugin_debug_error_printf)(const everything_plugin_utf8_t *format,...);
static unsigned short (EVERYTHING_PLUGIN_API *everything_plugin_os_winsock_ntohs)(unsigned short netshort);
static int (EVERYTHING_PLUGIN_API *everything_plugin_utf8_string_to_int)(const everything_plugin_utf8_t *str);
static void (EVERYTHING_PLUGIN_API *everything_plugin_utf8_string_parse_sockaddr_in)(const everything_plugin_utf8_t *s,struct everything_plugin_os_sockaddr_in *addr);
static void (EVERYTHING_PLUGIN_API *everything_plugin_utf8_string_parse_sockaddr_in6)(const everything_plugin_utf8_t *s,struct everything_plugin_os_winsock_sockaddr_in6 *addr);
static int (EVERYTHING_PLUGIN_API *everything_plugin_db_get_indexed_fd)(everything_plugin_db_t *db,const everything_plugin_utf8_t *filename,everything_plugin_fileinfo_fd_t *fd);
static void (EVERYTHING_PLUGIN_API *everything_plugin_ansi_buf_init)(everything_plugin_ansi_buf_t *acbuf);
static void (EVERYTHING_PLUGIN_API *everything_plugin_ansi_buf_kill)(everything_plugin_ansi_buf_t *acbuf);
static void (EVERYTHING_PLUGIN_API *everything_plugin_ansi_buf_copy_utf8_string)(everything_plugin_ansi_buf_t *acbuf,const everything_plugin_utf8_t *s);
static everything_plugin_utf8_t *(EVERYTHING_PLUGIN_API *everything_plugin_utf8_string_copy_utf8_string)(everything_plugin_utf8_t *buf,const everything_plugin_utf8_t *s);
static int (EVERYTHING_PLUGIN_API *everything_plugin_os_set_file_pointer)(HANDLE h,EVERYTHING_PLUGIN_QWORD position,int move_method);
static int (EVERYTHING_PLUGIN_API *everything_plugin_os_winsock_connect)(EVERYTHING_PLUGIN_OS_WINSOCK_SOCKET s,const struct everything_plugin_os_winsock_sockaddr *name,int namelen);
static int (EVERYTHING_PLUGIN_API *everything_plugin_db_is_index_folder_size)(everything_plugin_db_t *db);
static void (EVERYTHING_PLUGIN_API *everything_plugin_db_query_get_result_file_list_filename)(everything_plugin_db_query_t *q,uintptr_t index,everything_plugin_utf8_buf_t *cbuf);
static EVERYTHING_PLUGIN_QWORD (EVERYTHING_PLUGIN_API *everything_plugin_db_query_get_result_date_recently_changed)(everything_plugin_db_query_t *q,uintptr_t index);

// required procs supplied from Everything.
static etp_server_everything_plugin_proc_t etp_server_everything_plugin_proc_array[] =
{
 	{"mem_alloc",(void *)&everything_plugin_mem_alloc},
 	{"mem_calloc",(void *)&everything_plugin_mem_calloc},
 	{"mem_free",(void *)&everything_plugin_mem_free},
 	{"os_thread_wait_and_close",(void *)&everything_plugin_os_thread_wait_and_close},
 	{"os_winsock_closesocket",(void *)&everything_plugin_os_winsock_closesocket},
 	{"utf8_buf_init",(void *)&everything_plugin_utf8_buf_init},
 	{"utf8_buf_kill",(void *)&everything_plugin_utf8_buf_kill},
 	{"localization_get_string",(void *)&everything_plugin_localization_get_string},
 	{"utf8_buf_path_cat_filename",(void *)&everything_plugin_utf8_buf_path_cat_filename},
 	{"plugin_get_setting_string",(void *)&everything_plugin_get_setting_string},
 	{"plugin_get_setting_int",(void *)&everything_plugin_get_setting_int},
 	{"debug_printf",(void *)&everything_plugin_debug_printf},
 	{"utf8_buf_printf",(void *)&everything_plugin_utf8_buf_printf},
 	{"utf8_string_compare_nocase_s_sla",(void *)&everything_plugin_utf8_string_compare_nocase_s_sla},
 	{"os_open_file",(void *)&everything_plugin_os_open_file},
 	{"utf8_buf_copy_utf8_string",(void *)&everything_plugin_utf8_buf_copy_utf8_string},
 	{"utf8_buf_copy_utf8_string_n",(void *)&everything_plugin_utf8_buf_copy_utf8_string_n},
 	{"os_event_create",(void *)&everything_plugin_os_event_create},
 	{"os_thread_create",(void *)&everything_plugin_os_thread_create},
 	{"db_folder_exists",(void *)&everything_plugin_db_folder_exists},
 	{"db_file_exists",(void *)&everything_plugin_db_file_exists},
 	{"db_find_first_file",(void *)&everything_plugin_db_find_first_file},
 	{"db_find_next_file",(void *)&everything_plugin_db_find_next_file},
 	{"db_find_close",(void *)&everything_plugin_db_find_close},
 	{"safe_uintptr_add",(void *)&everything_plugin_safe_uintptr_add},
 	{"os_copy_memory",(void *)&everything_plugin_os_copy_memory},
 	{"debug_is_verbose",(void *)&everything_plugin_debug_is_verbose},
	{"utf8_string_realloc_utf8_string",(void *)&everything_plugin_utf8_string_realloc_utf8_string},
 	{"utf8_string_compare",(void *)&everything_plugin_utf8_string_compare},
 	{"utf8_string_to_dword",(void *)&everything_plugin_utf8_string_to_dword},
 	{"utf8_buf_path_canonicalize",(void *)&everything_plugin_utf8_buf_path_canonicalize},
 	{"utf8_buf_vprintf",(void *)&everything_plugin_utf8_buf_vprintf},
 	{"debug_color_printf",(void *)&everything_plugin_debug_color_printf},
	{"os_get_local_app_data_path_cat_make_filename",(void *)&everything_plugin_os_get_local_app_data_path_cat_make_filename},
 	{"os_get_local_app_data_path_cat_filename",(void *)&everything_plugin_os_get_local_app_data_path_cat_filename},
 	{"os_resize_file",(void *)&everything_plugin_os_resize_file},
 	{"os_make_sure_path_to_file_exists",(void *)&everything_plugin_os_make_sure_path_to_file_exists},
 	{"output_stream_append_file",(void *)&everything_plugin_output_stream_append_file},
 	{"output_stream_close",(void *)&everything_plugin_output_stream_close},
 	{"os_get_system_time_as_file_time",(void *)&everything_plugin_os_get_system_time_as_file_time},
 	{"version_get_text",(void *)&everything_plugin_version_get_text},
 	{"utf8_buf_format_filetime",(void *)&everything_plugin_utf8_buf_format_filetime},
 	{"output_stream_write_printf",(void *)&everything_plugin_output_stream_write_printf},
 	{"utf8_buf_format_peername",(void *)&everything_plugin_utf8_buf_format_peername},
 	{"utf8_buf_grow_length",(void *)&everything_plugin_utf8_buf_grow_length},
 	{"os_filetime_to_localtime",(void *)&everything_plugin_os_filetime_to_localtime},
 	{"os_zero_memory",(void *)&everything_plugin_os_zero_memory},
 	{"os_winsock_getaddrinfo",(void *)&everything_plugin_os_winsock_getaddrinfo},
 	{"os_winsock_socket",(void *)&everything_plugin_os_winsock_socket},
 	{"os_winsock_bind",(void *)&everything_plugin_os_winsock_bind},
 	{"os_winsock_listen",(void *)&everything_plugin_os_winsock_listen},
 	{"os_winsock_WSAAsyncSelect",(void *)&everything_plugin_os_winsock_WSAAsyncSelect},
 	{"os_winsock_WSAGetLastError",(void *)&everything_plugin_os_winsock_WSAGetLastError},
 	{"os_winsock_freeaddrinfo",(void *)&everything_plugin_os_winsock_freeaddrinfo},
 	{"os_winsock_WSAStartup",(void *)&everything_plugin_os_winsock_WSAStartup},
 	{"os_winsock_WSACleanup",(void *)&everything_plugin_os_winsock_WSACleanup},
 	{"utf8_string_alloc_utf8_string",(void *)&everything_plugin_utf8_string_alloc_utf8_string},
 	{"db_add_local_ref",(void *)&everything_plugin_db_add_local_ref},
 	{"db_release",(void *)&everything_plugin_db_release},
 	{"db_query_create",(void *)&everything_plugin_db_query_create},
 	{"db_query_destroy",(void *)&everything_plugin_db_query_destroy},
 	{"os_register_class",(void *)&everything_plugin_os_register_class},
 	{"os_create_window",(void *)&everything_plugin_os_create_window},
 	{"utf8_string_parse_csv_item",(void *)&everything_plugin_utf8_string_parse_csv_item},
 	{"ui_task_dialog_show",(void *)&everything_plugin_ui_task_dialog_show},
 	{"db_query_get_result_count",(void *)&everything_plugin_db_query_get_result_count},
 	{"db_query_get_result_name",(void *)&everything_plugin_db_query_get_result_name},
 	{"db_query_get_result_path",(void *)&everything_plugin_db_query_get_result_path},
 	{"db_query_get_result_indexed_fd",(void *)&everything_plugin_db_query_get_result_indexed_fd},
 	{"db_query_is_folder_result",(void *)&everything_plugin_db_query_is_folder_result},
 	{"os_winsock_accept",(void *)&everything_plugin_os_winsock_accept},
 	{"network_set_tcp_nodelay",(void *)&everything_plugin_network_set_tcp_nodelay},
 	{"network_set_keepalive",(void *)&everything_plugin_network_set_keepalive},
 	{"network_recv",(void *)&everything_plugin_network_recv},
 	{"network_send",(void *)&everything_plugin_network_send},
 	{"os_move_memory",(void *)&everything_plugin_os_move_memory},
 	{"os_winsock_shutdown",(void *)&everything_plugin_os_winsock_shutdown},
 	{"utf8_string_get_length_in_bytes",(void *)&everything_plugin_utf8_string_get_length_in_bytes},
 	{"property_get_builtin_type",(void *)&everything_plugin_property_get_builtin_type},
 	{"db_query_sort",(void *)&everything_plugin_db_query_sort},
 	{"db_query_search2",(void *)&everything_plugin_db_query_search2},
 	{"ui_options_add_plugin_page",(void *)&everything_plugin_ui_options_add_plugin_page},
 	{"plugin_set_setting_int",(void *)&everything_plugin_set_setting_int},
 	{"plugin_set_setting_string",(void *)&everything_plugin_set_setting_string},
 	{"os_get_logical_wide",(void *)&everything_plugin_os_get_logical_wide},
 	{"os_get_logical_high",(void *)&everything_plugin_os_get_logical_high},
 	{"os_set_dlg_rect",(void *)&everything_plugin_os_set_dlg_rect},
 	{"os_set_dlg_text",(void *)&everything_plugin_os_set_dlg_text},
 	{"os_get_dlg_text",(void *)&everything_plugin_os_get_dlg_text},
	{"os_enable_or_disable_dlg_item",(void *)&everything_plugin_os_enable_or_disable_dlg_item},
 	{"os_get_save_file_name",(void *)&everything_plugin_os_get_save_file_name},
 	{"os_create_checkbox",(void *)&everything_plugin_os_create_checkbox},
 	{"os_add_tooltip",(void *)&everything_plugin_os_add_tooltip},
 	{"os_create_static",(void *)&everything_plugin_os_create_static},
 	{"os_create_edit",(void *)&everything_plugin_os_create_edit},
 	{"os_create_number_edit",(void *)&everything_plugin_os_create_number_edit},
 	{"os_create_password_edit",(void *)&everything_plugin_os_create_password_edit},
 	{"os_create_button",(void *)&everything_plugin_os_create_button},
 	{"os_expand_dialog_text_logical_wide_no_prefix",(void *)&everything_plugin_os_expand_dialog_text_logical_wide_no_prefix},
 	{"os_winsock_getpeername",(void *)&everything_plugin_os_winsock_getpeername},
 	{"os_winsock_getsockname",(void *)&everything_plugin_os_winsock_getsockname},
 	{"unicode_is_ascii_ws",(void *)&everything_plugin_unicode_is_ascii_ws},
 	{"utf8_string_to_qword",(void *)&everything_plugin_utf8_string_to_qword},
 	{"debug_error_printf",(void *)&everything_plugin_debug_error_printf},
 	{"os_winsock_ntohs",(void *)&everything_plugin_os_winsock_ntohs},
 	{"utf8_string_to_int",(void *)&everything_plugin_utf8_string_to_int},
 	{"utf8_string_parse_sockaddr_in",(void *)&everything_plugin_utf8_string_parse_sockaddr_in},
 	{"utf8_string_parse_sockaddr_in6",(void *)&everything_plugin_utf8_string_parse_sockaddr_in6},
 	{"db_get_indexed_fd",(void *)&everything_plugin_db_get_indexed_fd},
 	{"ansi_buf_init",(void *)&everything_plugin_ansi_buf_init},
 	{"ansi_buf_kill",(void *)&everything_plugin_ansi_buf_kill},
 	{"ansi_buf_copy_utf8_string",(void *)&everything_plugin_ansi_buf_copy_utf8_string},
 	{"utf8_string_copy_utf8_string",(void *)&everything_plugin_utf8_string_copy_utf8_string},
 	{"os_set_file_pointer",(void *)&everything_plugin_os_set_file_pointer},
 	{"os_winsock_connect",(void *)&everything_plugin_os_winsock_connect},
 	{"db_is_index_folder_size",(void *)&everything_plugin_db_is_index_folder_size},
 	{"db_query_get_result_file_list_filename",(void *)&everything_plugin_db_query_get_result_file_list_filename},
 	{"db_query_get_result_date_recently_changed",(void *)&everything_plugin_db_query_get_result_date_recently_changed},
};
	
#define ETP_SERVER_EVERYTHING_PLUGIN_PROC_COUNT (sizeof(etp_server_everything_plugin_proc_array) / sizeof(etp_server_everything_plugin_proc_t))

__declspec( dllexport) void * EVERYTHING_PLUGIN_API everything_plugin_proc(DWORD msg,void *data)
{
	switch(msg)
	{
		case EVERYTHING_PLUGIN_PM_INIT:
		
			// find procs.
			
			{
				uintptr_t index;
				
				for(index=0;index<ETP_SERVER_EVERYTHING_PLUGIN_PROC_COUNT;index++)
				{
					void *proc;
					
					proc = ((everything_plugin_get_proc_address_t)data)(etp_server_everything_plugin_proc_array[index].name);
					
					if (!proc)
					{
						return (void *)0;
					}
					
					*etp_server_everything_plugin_proc_array[index].proc_address_ptr = proc;
				}
			}
						
			etp_server_welcome_message = everything_plugin_utf8_string_alloc_utf8_string((const everything_plugin_utf8_t *)"");
			etp_server_username = everything_plugin_utf8_string_alloc_utf8_string((const everything_plugin_utf8_t *)"");
			etp_server_password = everything_plugin_utf8_string_alloc_utf8_string((const everything_plugin_utf8_t *)"");
			etp_server_log_file_name = everything_plugin_utf8_string_alloc_utf8_string((const everything_plugin_utf8_t *)"");
			etp_server_bindings = everything_plugin_utf8_string_alloc_utf8_string((const everything_plugin_utf8_t *)"");
	
			return (void *)1;
			
		case EVERYTHING_PLUGIN_PM_START:
		
			// load settings
			etp_server_enabled = everything_plugin_get_setting_int(data,(const everything_plugin_utf8_t *)"enabled",etp_server_enabled);
			etp_server_welcome_message = everything_plugin_get_setting_string(data,(const everything_plugin_utf8_t *)"welcome_message",etp_server_welcome_message);
			etp_server_port = everything_plugin_get_setting_int(data,(const everything_plugin_utf8_t *)"port",etp_server_port);
			etp_server_username = everything_plugin_get_setting_string(data,(const everything_plugin_utf8_t *)"username",etp_server_username);
			etp_server_password = everything_plugin_get_setting_string(data,(const everything_plugin_utf8_t *)"password",etp_server_password);
			etp_server_log_file_name = everything_plugin_get_setting_string(data,(const everything_plugin_utf8_t *)"log_file_name",etp_server_log_file_name);
			etp_server_logging_enabled = everything_plugin_get_setting_int(data,(const everything_plugin_utf8_t *)"logging_enabled",etp_server_logging_enabled);
			etp_server_log_max_size = everything_plugin_get_setting_int(data,(const everything_plugin_utf8_t *)"log_max_size",etp_server_log_max_size);
			etp_server_log_delta_size = everything_plugin_get_setting_int(data,(const everything_plugin_utf8_t *)"log_delta_size",etp_server_log_delta_size);
			etp_server_allow_file_download = everything_plugin_get_setting_int(data,(const everything_plugin_utf8_t *)"allow_file_download",etp_server_allow_file_download);
			etp_server_bindings = everything_plugin_get_setting_string(data,(const everything_plugin_utf8_t *)"bindings",etp_server_bindings);
			etp_server_allow_ftp_port = everything_plugin_get_setting_int(data,(const everything_plugin_utf8_t *)"allow_ftp_port",etp_server_allow_ftp_port);
			etp_server_check_ftp_data_connection_ip = everything_plugin_get_setting_int(data,(const everything_plugin_utf8_t *)"check_ftp_data_connection_ip",etp_server_check_ftp_data_connection_ip);
			etp_server_allow_disk_access = everything_plugin_get_setting_int(data,(const everything_plugin_utf8_t *)"allow_disk_access",etp_server_allow_disk_access);

			// apply settings.
			etp_server_apply_settings();
			return (void *)1;
			
		case EVERYTHING_PLUGIN_PM_STOP:
			etp_server_shutdown();
			return (void *)1;
			
		case EVERYTHING_PLUGIN_PM_KILL:
		
			everything_plugin_mem_free(etp_server_bindings);
			everything_plugin_mem_free(etp_server_log_file_name);
			everything_plugin_mem_free(etp_server_password);
			everything_plugin_mem_free(etp_server_username);
			everything_plugin_mem_free(etp_server_welcome_message);
			return (void *)1;
			
		case EVERYTHING_PLUGIN_PM_GET_PLUGIN_VERSION:
			return (void *)EVERYTHING_PLUGIN_VERSION;
			
		case EVERYTHING_PLUGIN_PM_GET_NAME:
			return (void *)everything_plugin_localization_get_string(EVERYTHING_PLUGIN_LOCALIZATION_ETP_SERVER);
			
		case EVERYTHING_PLUGIN_PM_GET_DESCRIPTION:
			return (void *)everything_plugin_localization_get_string(EVERYTHING_PLUGIN_LOCALIZATION_ETP_SERVER_DESCRIPTION);
			
		case EVERYTHING_PLUGIN_PM_GET_AUTHOR:
			return "voidtools";
			
		case EVERYTHING_PLUGIN_PM_GET_VERSION:
			return PLUGINVERSION;
			
		case EVERYTHING_PLUGIN_PM_GET_LINK:
			return (void *)everything_plugin_localization_get_string(EVERYTHING_PLUGIN_LOCALIZATION_PLUGIN_LINK);
			
		case EVERYTHING_PLUGIN_PM_ADD_OPTIONS_PAGES:

			everything_plugin_ui_options_add_plugin_page(data,NULL,everything_plugin_localization_get_string(EVERYTHING_PLUGIN_LOCALIZATION_ETP_SERVER));
			
			return (void *)1;
	
		case EVERYTHING_PLUGIN_PM_LOAD_OPTIONS_PAGE:
		
			{
				HWND page_hwnd;
				
				page_hwnd = ((everything_plugin_load_options_page_t *)data)->page_hwnd;

				etp_server_create_checkbox(data,ETP_SERVER_PLUGIN_ID_ENABLED_CHECKBOX,WS_GROUP,EVERYTHING_PLUGIN_LOCALIZATION_ENABLE_ETP_SERVER,EVERYTHING_PLUGIN_LOCALIZATION_ENABLE_ETP_SERVER_HELP,etp_server_enabled);
				etp_server_create_static(data,ETP_SERVER_PLUGIN_ID_BINDINGS_STATIC,EVERYTHING_PLUGIN_LOCALIZATION_ETP_SERVER_BINDINGS);
				etp_server_create_edit(data,ETP_SERVER_PLUGIN_ID_BINDINGS_EDIT,EVERYTHING_PLUGIN_LOCALIZATION_ETP_SERVER_BINDINGS_HELP,etp_server_bindings);
				etp_server_create_static(data,ETP_SERVER_PLUGIN_ID_PORT_STATIC,EVERYTHING_PLUGIN_LOCALIZATION_ETP_SERVER_PORT);
				etp_server_create_number_edit(data,ETP_SERVER_PLUGIN_ID_PORT_EDIT,EVERYTHING_PLUGIN_LOCALIZATION_ETP_SERVER_PORT_HELP,etp_server_port);
				etp_server_create_static(data,ETP_SERVER_PLUGIN_ID_USERNAME_STATIC,EVERYTHING_PLUGIN_LOCALIZATION_ETP_SERVER_USERNAME);
				etp_server_create_edit(data,ETP_SERVER_PLUGIN_ID_USERNAME_EDIT,EVERYTHING_PLUGIN_LOCALIZATION_ETP_SERVER_USERNAME_HELP,etp_server_username);
				etp_server_create_static(data,ETP_SERVER_PLUGIN_ID_PASSWORD_STATIC,EVERYTHING_PLUGIN_LOCALIZATION_ETP_SERVER_PASSWORD);
				etp_server_create_password_edit(data,ETP_SERVER_PLUGIN_ID_PASSWORD_EDIT,EVERYTHING_PLUGIN_LOCALIZATION_ETP_SERVER_PASSWORD_HELP,etp_server_password);
				etp_server_create_checkbox(data,ETP_SERVER_PLUGIN_ID_LOGGING_ENABLED_CHECKBOX,WS_GROUP,EVERYTHING_PLUGIN_LOCALIZATION_ENABLE_ETP_SERVER_LOGGING,EVERYTHING_PLUGIN_LOCALIZATION_ENABLE_ETP_SERVER_LOGGING_HELP,etp_server_logging_enabled);
				etp_server_create_static(data,ETP_SERVER_PLUGIN_ID_LOG_FILE_STATIC,EVERYTHING_PLUGIN_LOCALIZATION_ETP_LOG_FILE);
				etp_server_create_edit(data,ETP_SERVER_PLUGIN_ID_LOG_FILE_NAME_EDIT,EVERYTHING_PLUGIN_LOCALIZATION_ETP_LOG_FILE_HELP,etp_server_log_file_name);
				etp_server_create_button(data,ETP_SERVER_PLUGIN_ID_LOG_FILE_NAME_BROWSE_BUTTON,WS_GROUP,EVERYTHING_PLUGIN_LOCALIZATION_ETP_SERVER_LOG_FILE_BROWSE,EVERYTHING_PLUGIN_LOCALIZATION_BROWSE_FOR_THE_ETP_SERVER_LOG_FILE);
				etp_server_create_static(data,ETP_SERVER_PLUGIN_ID_MAX_SIZE_STATIC,EVERYTHING_PLUGIN_LOCALIZATION_ETP_SERVER_MAX_LOG_SIZE);
				etp_server_create_number_edit(data,ETP_SERVER_PLUGIN_ID_LOG_MAX_SIZE_EDIT,EVERYTHING_PLUGIN_LOCALIZATION_ETP_SERVER_LOG_MAX_SIZE_HELP,(etp_server_log_max_size + 1023) / 1024);
				etp_server_create_static(data,ETP_SERVER_PLUGIN_ID_KB_STATIC,EVERYTHING_PLUGIN_LOCALIZATION_KB);
				etp_server_create_checkbox(data,ETP_SERVER_PLUGIN_ID_ALLOW_FILE_DOWNLOAD_CHECKBOX,WS_GROUP,EVERYTHING_PLUGIN_LOCALIZATION_ETP_SERVER_ALLOW_FILE_DOWNLOAD,EVERYTHING_PLUGIN_LOCALIZATION_ETP_SERVER_ALLOW_FILE_DOWNLOAD_HELP,etp_server_allow_file_download);
				etp_server_create_button(data,ETP_SERVER_PLUGIN_ID_RESTORE_DEFAULTS_BUTTON,WS_GROUP,EVERYTHING_PLUGIN_LOCALIZATION_ETP_SERVER_RESTORE_DEFAULTS,EVERYTHING_PLUGIN_LOCALIZATION_ETP_SERVER_RESTORE_DEFAULTS_HELP);

				etp_server_update_options_page(page_hwnd);
			}
			
			return (void *)1;
			
		case EVERYTHING_PLUGIN_PM_SAVE_OPTIONS_PAGE:

			{
				HWND page_hwnd;
				
				page_hwnd = ((everything_plugin_save_options_page_t *)data)->page_hwnd;
				
				etp_server_bindings = etp_server_get_options_text(page_hwnd,ETP_SERVER_PLUGIN_ID_BINDINGS_EDIT,etp_server_bindings);
				etp_server_port = GetDlgItemInt(page_hwnd,ETP_SERVER_PLUGIN_ID_PORT_EDIT,NULL,FALSE);
				etp_server_username = etp_server_get_options_text(page_hwnd,ETP_SERVER_PLUGIN_ID_USERNAME_EDIT,etp_server_username);
				etp_server_password = etp_server_get_options_text(page_hwnd,ETP_SERVER_PLUGIN_ID_PASSWORD_EDIT,etp_server_password);
				etp_server_logging_enabled = (IsDlgButtonChecked(page_hwnd,ETP_SERVER_PLUGIN_ID_LOGGING_ENABLED_CHECKBOX) == BST_CHECKED);
				etp_server_allow_file_download = (IsDlgButtonChecked(page_hwnd,ETP_SERVER_PLUGIN_ID_ALLOW_FILE_DOWNLOAD_CHECKBOX) == BST_CHECKED);
				etp_server_enabled = (IsDlgButtonChecked(page_hwnd,ETP_SERVER_PLUGIN_ID_ENABLED_CHECKBOX) == BST_CHECKED);
				etp_server_log_file_name = etp_server_get_options_text(page_hwnd,ETP_SERVER_PLUGIN_ID_LOG_FILE_NAME_EDIT,etp_server_log_file_name);
				etp_server_log_max_size = GetDlgItemInt(page_hwnd,ETP_SERVER_PLUGIN_ID_LOG_MAX_SIZE_EDIT,NULL,FALSE) * 1024;
				
				// restart servers?
				// why not ask the user..
				if (!etp_server_apply_settings())
				{
					((everything_plugin_save_options_page_t *)data)->enable_apply = 1;
				}
			}
			
			return (void *)1;
				
		case EVERYTHING_PLUGIN_PM_SAVE_SETTINGS:
			
			// save settings
			everything_plugin_set_setting_int(data,(const everything_plugin_utf8_t *)"enabled",etp_server_enabled);
			everything_plugin_set_setting_string(data,(const everything_plugin_utf8_t *)"welcome_message",etp_server_welcome_message);
			everything_plugin_set_setting_int(data,(const everything_plugin_utf8_t *)"port",etp_server_port);
			everything_plugin_set_setting_string(data,(const everything_plugin_utf8_t *)"username",etp_server_username);
			everything_plugin_set_setting_string(data,(const everything_plugin_utf8_t *)"password",etp_server_password);
			everything_plugin_set_setting_string(data,(const everything_plugin_utf8_t *)"log_file_name",etp_server_log_file_name);
			everything_plugin_set_setting_int(data,(const everything_plugin_utf8_t *)"logging_enabled",etp_server_logging_enabled);
			everything_plugin_set_setting_int(data,(const everything_plugin_utf8_t *)"log_max_size",etp_server_log_max_size);
			everything_plugin_set_setting_int(data,(const everything_plugin_utf8_t *)"log_delta_size",etp_server_log_delta_size);
			everything_plugin_set_setting_int(data,(const everything_plugin_utf8_t *)"allow_file_download",etp_server_allow_file_download);
			everything_plugin_set_setting_string(data,(const everything_plugin_utf8_t *)"bindings",etp_server_bindings);
			everything_plugin_set_setting_int(data,(const everything_plugin_utf8_t *)"allow_ftp_port",etp_server_allow_ftp_port);
			everything_plugin_set_setting_int(data,(const everything_plugin_utf8_t *)"check_ftp_data_connection_ip",etp_server_check_ftp_data_connection_ip);
			everything_plugin_set_setting_int(data,(const everything_plugin_utf8_t *)"allow_disk_access",etp_server_allow_disk_access);
		
			return (void *)1;
		
		case EVERYTHING_PLUGIN_PM_GET_OPTIONS_PAGE_MINMAX:
			
			((everything_plugin_get_options_page_minmax_t *)data)->wide = 200;
			((everything_plugin_get_options_page_minmax_t *)data)->high = 298;
			return (void *)1;
			
		case EVERYTHING_PLUGIN_PM_SIZE_OPTIONS_PAGE:
		
			{
				HWND page_hwnd;
				int static_wide;
				int button_wide;
				RECT rect;
				int x;
				int y;
				int wide;
				int high;
				
				page_hwnd = ((everything_plugin_size_options_page_t *)data)->page_hwnd;
				GetClientRect(page_hwnd,&rect);
				wide = rect.right - rect.left;
				high = rect.bottom - rect.top;
	
				wide = (wide * 96) / everything_plugin_os_get_logical_wide();
				high = (high * 96) / everything_plugin_os_get_logical_high();

				x = 12;
				y = 12;
				wide -= 24;
				high -= 24;
			
				static_wide = 0;
				static_wide = etp_server_expand_min_wide(page_hwnd,EVERYTHING_PLUGIN_LOCALIZATION_ETP_SERVER_BINDINGS,static_wide);
				static_wide = etp_server_expand_min_wide(page_hwnd,EVERYTHING_PLUGIN_LOCALIZATION_ETP_SERVER_PORT,static_wide);
				static_wide = etp_server_expand_min_wide(page_hwnd,EVERYTHING_PLUGIN_LOCALIZATION_ETP_SERVER_USERNAME,static_wide);
				static_wide = etp_server_expand_min_wide(page_hwnd,EVERYTHING_PLUGIN_LOCALIZATION_ETP_SERVER_PASSWORD,static_wide);
				static_wide += 6;

				button_wide = 75 - 24;
				button_wide = etp_server_expand_min_wide(page_hwnd,EVERYTHING_PLUGIN_LOCALIZATION_ETP_SERVER_LOG_FILE_BROWSE,button_wide);
				button_wide += 24;

				everything_plugin_os_set_dlg_rect(page_hwnd,ETP_SERVER_PLUGIN_ID_ENABLED_CHECKBOX,x,y,wide,EVERYTHING_PLUGIN_OS_DLG_CHECKBOX_HIGH);
				y += EVERYTHING_PLUGIN_OS_DLG_CHECKBOX_HIGH + EVERYTHING_PLUGIN_OS_DLG_SEPARATOR;
				
				everything_plugin_os_set_dlg_rect(page_hwnd,ETP_SERVER_PLUGIN_ID_BINDINGS_STATIC,x,y+3,static_wide,EVERYTHING_PLUGIN_OS_DLG_STATIC_HIGH);
				everything_plugin_os_set_dlg_rect(page_hwnd,ETP_SERVER_PLUGIN_ID_BINDINGS_EDIT,x+static_wide,y,wide - (static_wide),EVERYTHING_PLUGIN_OS_DLG_EDIT_HIGH);
				y += EVERYTHING_PLUGIN_OS_DLG_EDIT_HIGH + EVERYTHING_PLUGIN_OS_DLG_SEPARATOR;
				
				everything_plugin_os_set_dlg_rect(page_hwnd,ETP_SERVER_PLUGIN_ID_PORT_STATIC,x,y+3,static_wide,EVERYTHING_PLUGIN_OS_DLG_STATIC_HIGH);
				everything_plugin_os_set_dlg_rect(page_hwnd,ETP_SERVER_PLUGIN_ID_PORT_EDIT,x+static_wide,y,75,EVERYTHING_PLUGIN_OS_DLG_EDIT_HIGH);
				y += 27;
				
				everything_plugin_os_set_dlg_rect(page_hwnd,ETP_SERVER_PLUGIN_ID_USERNAME_STATIC,x,y+3,static_wide,EVERYTHING_PLUGIN_OS_DLG_STATIC_HIGH);
				everything_plugin_os_set_dlg_rect(page_hwnd,ETP_SERVER_PLUGIN_ID_USERNAME_EDIT,x+static_wide,y,wide-static_wide,EVERYTHING_PLUGIN_OS_DLG_EDIT_HIGH);
				y += EVERYTHING_PLUGIN_OS_DLG_EDIT_HIGH + EVERYTHING_PLUGIN_OS_DLG_SEPARATOR;

				everything_plugin_os_set_dlg_rect(page_hwnd,ETP_SERVER_PLUGIN_ID_PASSWORD_STATIC,x,y+3,static_wide,EVERYTHING_PLUGIN_OS_DLG_STATIC_HIGH);
				everything_plugin_os_set_dlg_rect(page_hwnd,ETP_SERVER_PLUGIN_ID_PASSWORD_EDIT,x+static_wide,y,wide-static_wide,EVERYTHING_PLUGIN_OS_DLG_EDIT_HIGH);
				y += EVERYTHING_PLUGIN_OS_DLG_EDIT_HIGH + EVERYTHING_PLUGIN_OS_DLG_SEPARATOR;

				everything_plugin_os_set_dlg_rect(page_hwnd,ETP_SERVER_PLUGIN_ID_LOGGING_ENABLED_CHECKBOX,x,y,wide,EVERYTHING_PLUGIN_OS_DLG_CHECKBOX_HIGH);
				y += EVERYTHING_PLUGIN_OS_DLG_CHECKBOX_HIGH + EVERYTHING_PLUGIN_OS_DLG_SEPARATOR;

				everything_plugin_os_set_dlg_rect(page_hwnd,ETP_SERVER_PLUGIN_ID_LOG_FILE_STATIC,x,y,wide,EVERYTHING_PLUGIN_OS_DLG_STATIC_HIGH);
				y += EVERYTHING_PLUGIN_OS_DLG_STATIC_HIGH + EVERYTHING_PLUGIN_OS_DLG_STATIC_SEPARATOR;
				
				everything_plugin_os_set_dlg_rect(page_hwnd,ETP_SERVER_PLUGIN_ID_LOG_FILE_NAME_EDIT,x,y+1,wide-button_wide - 6,EVERYTHING_PLUGIN_OS_DLG_EDIT_HIGH);
				everything_plugin_os_set_dlg_rect(page_hwnd,ETP_SERVER_PLUGIN_ID_LOG_FILE_NAME_BROWSE_BUTTON,x+wide-button_wide,y,button_wide,EVERYTHING_PLUGIN_OS_DLG_BUTTON_HIGH);
				y += EVERYTHING_PLUGIN_OS_DLG_BUTTON_HIGH + EVERYTHING_PLUGIN_OS_DLG_SEPARATOR;
				
				static_wide = 0;
				static_wide = etp_server_expand_min_wide(page_hwnd,EVERYTHING_PLUGIN_LOCALIZATION_ETP_SERVER_MAX_LOG_SIZE,static_wide);
				static_wide += 6;
				
				everything_plugin_os_set_dlg_rect(page_hwnd,ETP_SERVER_PLUGIN_ID_MAX_SIZE_STATIC,x,y+3,static_wide,EVERYTHING_PLUGIN_OS_DLG_STATIC_HIGH);
				everything_plugin_os_set_dlg_rect(page_hwnd,ETP_SERVER_PLUGIN_ID_LOG_MAX_SIZE_EDIT,x+static_wide,y,75,EVERYTHING_PLUGIN_OS_DLG_EDIT_HIGH);
				everything_plugin_os_set_dlg_rect(page_hwnd,ETP_SERVER_PLUGIN_ID_KB_STATIC,x+static_wide+75+7,y+3,static_wide,EVERYTHING_PLUGIN_OS_DLG_STATIC_HIGH);
				y += 27;
				
				everything_plugin_os_set_dlg_rect(page_hwnd,ETP_SERVER_PLUGIN_ID_ALLOW_FILE_DOWNLOAD_CHECKBOX,x,y,wide,EVERYTHING_PLUGIN_OS_DLG_CHECKBOX_HIGH);
				y += EVERYTHING_PLUGIN_OS_DLG_CHECKBOX_HIGH + 12;
				
				// restore defaults
				button_wide = 75 - 24;
				button_wide = etp_server_expand_min_wide(page_hwnd,EVERYTHING_PLUGIN_LOCALIZATION_ETP_SERVER_RESTORE_DEFAULTS,button_wide);
				button_wide += 24;
					
				everything_plugin_os_set_dlg_rect(page_hwnd,ETP_SERVER_PLUGIN_ID_RESTORE_DEFAULTS_BUTTON,x + wide - button_wide,12 + high - EVERYTHING_PLUGIN_OS_DLG_BUTTON_HIGH,button_wide,EVERYTHING_PLUGIN_OS_DLG_BUTTON_HIGH);
			}
			
			return (void *)1;

		case EVERYTHING_PLUGIN_PM_OPTIONS_PAGE_PROC:
		
			{
				HWND page_hwnd;
				
				page_hwnd = ((everything_plugin_options_page_proc_t *)data)->page_hwnd;
				
				switch(((everything_plugin_options_page_proc_t *)data)->msg)
				{
					case WM_COMMAND:
					
						switch(LOWORD(((everything_plugin_options_page_proc_t *)data)->wParam))
						{
							case ETP_SERVER_PLUGIN_ID_ALLOW_FILE_DOWNLOAD_CHECKBOX:

								etp_server_enable_options_apply(data);
							
								break;
							
							case ETP_SERVER_PLUGIN_ID_LOGGING_ENABLED_CHECKBOX:
							case ETP_SERVER_PLUGIN_ID_ENABLED_CHECKBOX:
								etp_server_update_options_page(page_hwnd);
								etp_server_enable_options_apply(data);
								break;
								
							case ETP_SERVER_PLUGIN_ID_LOG_FILE_NAME_EDIT:
							case ETP_SERVER_PLUGIN_ID_PORT_EDIT:
							case ETP_SERVER_PLUGIN_ID_BINDINGS_EDIT:
							case ETP_SERVER_PLUGIN_ID_LOG_MAX_SIZE_EDIT:
							case ETP_SERVER_PLUGIN_ID_USERNAME_EDIT:
							case ETP_SERVER_PLUGIN_ID_PASSWORD_EDIT:
							
								if (HIWORD(((everything_plugin_options_page_proc_t *)data)->wParam) == EN_CHANGE)
								{
									etp_server_enable_options_apply(data);
								}

								break;

							case ETP_SERVER_PLUGIN_ID_RESTORE_DEFAULTS_BUTTON:
							
								CheckDlgButton(page_hwnd,ETP_SERVER_PLUGIN_ID_ENABLED_CHECKBOX,BST_UNCHECKED);
								CheckDlgButton(page_hwnd,ETP_SERVER_PLUGIN_ID_LOGGING_ENABLED_CHECKBOX,BST_CHECKED);
								CheckDlgButton(page_hwnd,ETP_SERVER_PLUGIN_ID_ALLOW_FILE_DOWNLOAD_CHECKBOX,BST_CHECKED);

								everything_plugin_os_set_dlg_text(page_hwnd,ETP_SERVER_PLUGIN_ID_LOG_FILE_NAME_EDIT,(const everything_plugin_utf8_t *)"");
								everything_plugin_os_set_dlg_text(page_hwnd,ETP_SERVER_PLUGIN_ID_USERNAME_EDIT,(const everything_plugin_utf8_t *)"");
								everything_plugin_os_set_dlg_text(page_hwnd,ETP_SERVER_PLUGIN_ID_PASSWORD_EDIT,(const everything_plugin_utf8_t *)"");
								everything_plugin_os_set_dlg_text(page_hwnd,ETP_SERVER_PLUGIN_ID_BINDINGS_EDIT,(const everything_plugin_utf8_t *)"");

								SetDlgItemInt(page_hwnd,ETP_SERVER_PLUGIN_ID_PORT_EDIT,ETP_SERVER_DEFAULT_PORT,FALSE);
								SetDlgItemInt(page_hwnd,ETP_SERVER_PLUGIN_ID_LOG_MAX_SIZE_EDIT,(ETP_SERVER_DEFAULT_LOG_MAX_SIZE + 1023) / 1024,FALSE);

								etp_server_update_options_page(page_hwnd);
								etp_server_enable_options_apply(data);
								break;
							
							case ETP_SERVER_PLUGIN_ID_LOG_FILE_NAME_BROWSE_BUTTON:
						
								{
									everything_plugin_utf8_buf_t absfilename_cbuf;
									
									everything_plugin_utf8_buf_init(&absfilename_cbuf);

									{
										everything_plugin_utf8_buf_t filename_cbuf;
										
										everything_plugin_utf8_buf_init(&filename_cbuf);
								
										everything_plugin_os_get_dlg_text(page_hwnd,ETP_SERVER_PLUGIN_ID_LOG_FILE_NAME_EDIT,&filename_cbuf);
										
										if (*filename_cbuf.buf)
										{
											everything_plugin_os_get_local_app_data_path_cat_filename(filename_cbuf.buf,&absfilename_cbuf);
										}
										else
										{
											everything_plugin_os_get_local_app_data_path_cat_make_filename(*filename_cbuf.buf ? filename_cbuf.buf : (const everything_plugin_utf8_t *)"Logs\\ETP_Server_Log",(const everything_plugin_utf8_t *)".txt",&absfilename_cbuf);
										}
										
										everything_plugin_utf8_buf_kill(&filename_cbuf);
									}

									{			
										everything_plugin_utf8_buf_t filename_cbuf;
										everything_plugin_utf8_buf_t filter_cbuf;
										
										everything_plugin_utf8_buf_init(&filename_cbuf);
										everything_plugin_utf8_buf_init(&filter_cbuf);

										everything_plugin_utf8_buf_printf(&filter_cbuf,(const everything_plugin_utf8_t *)"%s (*.txt)%c*.txt%c%s (*.*)%c*.*%c%c",everything_plugin_localization_get_string(EVERYTHING_PLUGIN_LOCALIZATION_TEXT_FILES),0,0,everything_plugin_localization_get_string(EVERYTHING_PLUGIN_LOCALIZATION_ALL_FILES),0,0,0);
										
										if (everything_plugin_os_get_save_file_name(((everything_plugin_options_page_proc_t *)data)->options_hwnd,everything_plugin_localization_get_string(EVERYTHING_PLUGIN_LOCALIZATION_SAVE_ETP_SERVER_LOG_FILE),absfilename_cbuf.buf,filter_cbuf.buf,filter_cbuf.len,1,(const everything_plugin_utf8_t *)"txt",NULL,&filename_cbuf))
										{	
											everything_plugin_os_set_dlg_text(page_hwnd,ETP_SERVER_PLUGIN_ID_LOG_FILE_NAME_EDIT,filename_cbuf.buf);
										}	

										everything_plugin_utf8_buf_kill(&filter_cbuf);
										everything_plugin_utf8_buf_kill(&filename_cbuf);
									}

									everything_plugin_utf8_buf_kill(&absfilename_cbuf);
								}
								
								break;
						}
					
						break;
					
				}
			}
			
			return (void *)1;
	}
	
	return 0;
}

// disconect
static void etp_server_shutdown(void)
{
	if (_etp_server)
	{
		// destroy listening sockets.
		{
			etp_server_listen_t *l;
			etp_server_listen_t *next_l;

			l = _etp_server->listen_start;
			while(l)			
			{
				next_l = l->next;
				
				everything_plugin_os_winsock_closesocket(l->listen_socket);
				everything_plugin_mem_free(l);
				
				l = next_l;
			}
		}
		
		// delete the clients.
		{
			etp_server_client_t *c,*next_c;
			
			c = _etp_server->client_start;
			while(c)
			{
				next_c = c->next;

				etp_server_client_destroy(c);
				
				c = next_c;
			}
		}
	
		etp_server_log(0,(const everything_plugin_utf8_t *)"ETP server offline.\r\n");

		everything_plugin_db_release(_etp_server->db);

		// server user
		everything_plugin_mem_free(_etp_server->bindings);

		// there MUST be a WSACleanup
		everything_plugin_os_winsock_WSACleanup();
		
		DestroyWindow(_etp_server->hwnd);
		
		if (_etp_server->log_file)
		{
			everything_plugin_output_stream_close(_etp_server->log_file);
		}
		
		everything_plugin_mem_free(_etp_server);
		
		_etp_server = 0;
	}
}

// insert the client
static etp_server_client_t *etp_server_client_create(EVERYTHING_PLUGIN_OS_WINSOCK_SOCKET socket_handle)
{
	etp_server_client_t *c;
	
	c = everything_plugin_mem_calloc(sizeof(etp_server_client_t));
	
	c->control_socket = socket_handle;
	c->login_state = ETP_SERVER_CLIENT_LOGIN_STATE_NEED_USER;
	c->db_query = everything_plugin_db_query_create(_etp_server->db,etp_server_db_query_event_proc,c);
	c->working_directory = everything_plugin_utf8_string_alloc_utf8_string((const everything_plugin_utf8_t *)"");
	c->data_socket = EVERYTHING_PLUGIN_OS_WINSOCK_INVALID_SOCKET;
	c->is_ipv6 = 0;
	c->data_type = ETP_SERVER_CLIENT_DATA_TYPE_NONE;
	c->search_string = everything_plugin_utf8_string_alloc_utf8_string((const everything_plugin_utf8_t *)"");
	c->filter_search = everything_plugin_utf8_string_alloc_utf8_string((const everything_plugin_utf8_t *)"");
	c->sort_property_type = everything_plugin_property_get_builtin_type(EVERYTHING_PLUGIN_PROPERTY_TYPE_NAME);
	c->sort_ascending = 1;
	c->current_search_string = everything_plugin_utf8_string_alloc_utf8_string((const everything_plugin_utf8_t *)"");
	c->current_filter_search = everything_plugin_utf8_string_alloc_utf8_string((const everything_plugin_utf8_t *)"");
	c->current_sort_column_type = everything_plugin_property_get_builtin_type(EVERYTHING_PLUGIN_PROPERTY_TYPE_NAME);
	c->current_sort_ascending = 1;
	c->utf8_on = 1;
	c->count = 0xffffffff;

	// get peer addr
	// make sure data addr matches control addr.
	{
		int addr_len;
		
		addr_len = sizeof(struct everything_plugin_os_winsock_sockaddr_storage);

		everything_plugin_os_winsock_getpeername(c->control_socket,(struct everything_plugin_os_winsock_sockaddr *)&c->control_addr,&addr_len);
	}


	// RFC959-ftp
	// The user-process default data port is the same as the
    // control connection port (i.e., U).  
	
	{
		struct everything_plugin_os_winsock_sockaddr_storage control_sockaddr;
		int sockaddr_size;
		
		sockaddr_size = sizeof(struct everything_plugin_os_winsock_sockaddr_storage);
		everything_plugin_os_winsock_getsockname(c->control_socket,(struct everything_plugin_os_winsock_sockaddr *)&control_sockaddr,&sockaddr_size);
		
		if (control_sockaddr.ss_family == EVERYTHING_PLUGIN_OS_WINSOCK_PF_INET6)
		{
			c->is_ipv6 = 1;
		}
	}
	
	if (_etp_server->client_start)
	{
		_etp_server->client_last->next = c;
		c->prev = _etp_server->client_last;
	}
	else
	{
		_etp_server->client_start = c;
		c->prev = 0;
	}
	
	c->next = 0;
	
	_etp_server->client_last = c;
	
	etp_server_log(c,(const everything_plugin_utf8_t *)"Connected.\r\n");
	
	return c;
}

// remove the client
static void etp_server_client_destroy(etp_server_client_t *c)
{
	// log disconnect.
	etp_server_log(c,(const everything_plugin_utf8_t *)"Disconnected.\r\n");

	// unlink.	
	if (c->prev)
	{
		c->prev->next = c->next;
	}
	else
	{
		_etp_server->client_start = c->next;
	}
	
	if (c->next)
	{
		c->next->prev = c->prev;
	}
	else
	{
		_etp_server->client_last = c->prev;
	}
	
	// free
	etp_server_close_data(c);
	
	if (c->control_socket != EVERYTHING_PLUGIN_OS_WINSOCK_INVALID_SOCKET)
	{
		everything_plugin_os_winsock_closesocket(c->control_socket);
	}
		
	everything_plugin_db_query_destroy(c->db_query);

	if (c->last_search_string)
	{
		everything_plugin_mem_free(c->last_search_string);
	}

	// free working directory.	
	everything_plugin_mem_free(c->working_directory);
	everything_plugin_mem_free(c->search_string);
	everything_plugin_mem_free(c->filter_search);
	everything_plugin_mem_free(c->current_search_string);
	everything_plugin_mem_free(c->current_filter_search);

	// destroy send packets
	{
		etp_server_send_packet_t *send_packet;
		etp_server_send_packet_t *next_send_packet;
		
		send_packet = c->send_start;
		while(send_packet)
		{
			next_send_packet = send_packet->next;
			
			etp_server_send_packet_destroy(send_packet);
			
			send_packet = next_send_packet;
		}
	}

	// destroy recv packets.
	{
		etp_server_recv_chunk_t *recv_chunk;
		etp_server_recv_chunk_t *next_recv_chunk;
		
		recv_chunk = c->recv_chunk_start;
		
		while(recv_chunk)
		{
			next_recv_chunk = recv_chunk->next;
			
			etp_server_recv_chunk_destroy(recv_chunk);
			
			recv_chunk = next_recv_chunk;
		}
	}
	
	everything_plugin_mem_free(c);
}

static int etp_server_is_config_change(void)
{
	if (_etp_server)
	{
		if (etp_server_port != _etp_server->port)
		{
			return 1;
		}

		if (everything_plugin_utf8_string_compare(etp_server_bindings,_etp_server->bindings) != 0)
		{
			return 1;
		}
	}
	
	return 0;
}

static int etp_server_apply_settings(void)
{
	if (etp_server_enabled)
	{
		if (etp_server_is_config_change())
		{
			etp_server_shutdown();
		}

		etp_server_start();
		
		if (_etp_server)
		{
			return 1;
		}
	}
	else
	{
		etp_server_shutdown();
		
		return 1;
	}
	
	return 0;
}

static etp_server_client_t *etp_server_client_find_control_socket(EVERYTHING_PLUGIN_OS_WINSOCK_SOCKET socket_handle)
{
	etp_server_client_t *c;
	
	c = _etp_server->client_start;
	
	while(c)
	{
		if (c->control_socket == socket_handle)
		{
			return c;
		}
	
		c = c->next;
	}
	
	return 0;
}

static etp_server_client_t *etp_server_client_find_pasv_socket(EVERYTHING_PLUGIN_OS_WINSOCK_SOCKET socket_handle)
{
	etp_server_client_t *c;
	
	c = _etp_server->client_start;
	
	while(c)
	{
		if (c->data_connection_type == ETP_SERVER_CLIENT_DATA_CONNECTION_TYPE_PASV)
		{
			if (c->data_connection_data.pasv.socket_handle == socket_handle)
			{
				return c;
			}
		}
	
		c = c->next;
	}
	
	return 0;
}

static etp_server_client_t *etp_server_client_find_data_socket(EVERYTHING_PLUGIN_OS_WINSOCK_SOCKET socket_handle)
{
	etp_server_client_t *c;
	
	c = _etp_server->client_start;
	
	while(c)
	{
		if (c->data_socket == socket_handle)
		{
			return c;
		}
	
		c = c->next;
	}
	
	return 0;
}

static LRESULT WINAPI etp_server_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch(msg)
	{
		// update listening sockets.
		case ETP_SERVER_WM_LISTEN:
		{
			etp_server_listen_t *l;

			if (everything_plugin_debug_is_verbose())
			{
				everything_plugin_debug_printf((const everything_plugin_utf8_t *)"listen event %zu %zu\n",wParam,lParam);
			}
			
			l = _etp_server->listen_start;
			while(l)
			{
				EVERYTHING_PLUGIN_OS_WINSOCK_SOCKET client_socket;
				
				// accept could fail with WSAEWOULDBLOCK because OS_WINSOCK_FD_ACCEPT could be an old post.
				client_socket = everything_plugin_os_winsock_accept(l->listen_socket,0,0);
				if (client_socket != EVERYTHING_PLUGIN_OS_WINSOCK_INVALID_SOCKET)
				{
					everything_plugin_network_set_tcp_nodelay(client_socket);
					everything_plugin_network_set_keepalive(client_socket);
											
					everything_plugin_os_winsock_WSAAsyncSelect(client_socket,hwnd,ETP_SERVER_WM_CLIENT,EVERYTHING_PLUGIN_OS_WINSOCK_FD_READ|EVERYTHING_PLUGIN_OS_WINSOCK_FD_WRITE|EVERYTHING_PLUGIN_OS_WINSOCK_FD_CLOSE);

					{				
						etp_server_client_t *c;

						c = etp_server_client_create(client_socket);
						
						{
							everything_plugin_utf8_buf_t message_cbuf;
							
							everything_plugin_utf8_buf_init(&message_cbuf);
							
							everything_plugin_utf8_buf_grow_length(&message_cbuf,(uintptr_t)etp_server_get_welcome_message(0,etp_server_welcome_message));
							
							etp_server_get_welcome_message(message_cbuf.buf,etp_server_welcome_message);
							
							etp_server_send_packet_add(c,message_cbuf.buf,message_cbuf.len);

							everything_plugin_utf8_buf_kill(&message_cbuf);
						}
					}
				}
				
				l = l->next;
			}

			break;
		}
			
		// update client socket
		case ETP_SERVER_WM_CLIENT:
		{
			etp_server_client_t *c;

			if (everything_plugin_debug_is_verbose())
			{
				everything_plugin_debug_printf((const everything_plugin_utf8_t *)"client event %zu %zu\n",wParam,lParam);
			}

			c = etp_server_client_find_control_socket((EVERYTHING_PLUGIN_OS_WINSOCK_SOCKET)wParam);
			if (c)
			{
				// try to read.
				// if it fails we know the socket has really been closed.
				// we HAVE to do this because OS_WINSOCK_FD_CLOSE could be posted 
				// to an old socket, that happens to be the same as this one.
				if (!etp_server_client_update_recv(c))
				{
				everything_plugin_debug_printf("recv failed\n");
					etp_server_client_destroy(c);
					
					break;
				}

				if (!etp_server_client_update_send(c))
				{
				everything_plugin_debug_printf("send failed\n");
					etp_server_client_destroy(c);
					
					break;
				}
			}
			
			break;
		}			

		case ETP_SERVER_WM_PASV:
		{
			etp_server_client_t *c;

			if (everything_plugin_debug_is_verbose())
			{
				everything_plugin_debug_printf((const everything_plugin_utf8_t *)"pasv event %zu %zu\n",wParam,lParam);
			}

			c = etp_server_client_find_pasv_socket((EVERYTHING_PLUGIN_OS_WINSOCK_SOCKET)wParam);
			if (c)
			{
				c->data_socket = everything_plugin_os_winsock_accept(c->data_connection_data.pasv.socket_handle,0,0);
				if (c->data_socket != EVERYTHING_PLUGIN_OS_WINSOCK_INVALID_SOCKET)
				{
					if (etp_server_validate_ftp_data_connection_ip(c->data_socket,(struct everything_plugin_os_winsock_sockaddr *)&c->control_addr))
					{
					
	everything_plugin_debug_printf((const everything_plugin_utf8_t *)"pasv connected.\n");

						everything_plugin_network_set_tcp_nodelay(c->data_socket);
						everything_plugin_network_set_keepalive(c->data_socket);

						// set nonblocking
						everything_plugin_os_winsock_WSAAsyncSelect(c->data_socket,_etp_server->hwnd,ETP_SERVER_WM_DATA,EVERYTHING_PLUGIN_OS_WINSOCK_FD_WRITE|EVERYTHING_PLUGIN_OS_WINSOCK_FD_READ|EVERYTHING_PLUGIN_OS_WINSOCK_FD_CLOSE);
						
						etp_server_run_data_command(c);
						
						// close pasv socket.
						everything_plugin_os_winsock_closesocket(c->data_connection_data.pasv.socket_handle);
						
						c->data_connection_data.pasv.socket_handle = EVERYTHING_PLUGIN_OS_WINSOCK_INVALID_SOCKET;
					}
					else
					{
						etp_server_client_printf(c,(const everything_plugin_utf8_t *)"500 Failed to connect to IP.\r\n",EVERYTHING_PLUGIN_OS_WSAGETSELECTERROR(lParam));

						etp_server_close_data(c);						
					}
				}
			}
			
			break;		
		}
	
		case ETP_SERVER_WM_DATA:
		{
			etp_server_client_t *c;
			
			if (everything_plugin_debug_is_verbose())
			{
				everything_plugin_debug_printf((const everything_plugin_utf8_t *)"data event %zu %zu\n",wParam,lParam);
			}

			c = etp_server_client_find_data_socket((EVERYTHING_PLUGIN_OS_WINSOCK_SOCKET)wParam);
			if (c)
			{
				// connect..
				if (!c->data_is_connected)
				{
					// this could be an old message...
					// but theres nothing we can do...
					// once we start to read from the socket we could get WSAENOTCONNECTED..
					if (EVERYTHING_PLUGIN_OS_WSAGETSELECTEVENT(lParam) == EVERYTHING_PLUGIN_OS_WINSOCK_FD_CONNECT)
					{
						if (EVERYTHING_PLUGIN_OS_WSAGETSELECTERROR(lParam) == 0)
						{
							etp_server_run_data_command(c);
						}
						else
						{
							etp_server_client_printf(c,(const everything_plugin_utf8_t *)"426 Unable to connect to port %u\r\n",EVERYTHING_PLUGIN_OS_WSAGETSELECTERROR(lParam));

							etp_server_close_data(c);
						}
					}
				}
				else
				{
					if (!etp_server_client_update_data(c))
					{
						if (c->data_complete)
						{
							everything_plugin_os_winsock_shutdown(c->data_socket,EVERYTHING_PLUGIN_OS_WINSOCK_SD_SEND);

							etp_server_close_data(c);

							etp_server_client_printf(c,(const everything_plugin_utf8_t *)"226 Transfer complete.\r\n");
						}
						else
						{
							etp_server_close_data(c);
						}
				
						break;
					}
				}
			}
			
			break;
		}
	}
	
	return DefWindowProc(hwnd,msg,wParam,lParam);
}

static void etp_server_client_process_command(etp_server_client_t *c,everything_plugin_utf8_t *command)
{
	everything_plugin_utf8_t *param;
	
	// remove any trailing '\r'
	{
		everything_plugin_utf8_t *p;
		
		p = command;
		
		while(*p)
		{
			if ((*p == '\r') && (!p[1]))
			{
				*p = 0;
				
				break;
			}
			
			p++;
		}
	}
	
	// find command and param parts
	{
		everything_plugin_utf8_t *p;
		
		p = command;
		
		while(*p)
		{
			if (everything_plugin_unicode_is_ascii_ws(*p)) 
			{
				*p++ = 0;

				break;
			}

			p++;
		}
		
		// dont skip any more white spaces
		// we want to be able to process filenames starting with spaces!
		param = p;
	}
	
	if (everything_plugin_debug_is_verbose())
	{
		everything_plugin_debug_color_printf(0xff0000ff,(const everything_plugin_utf8_t *)"%s %s\n",command,param);
	}

	etp_server_log(c,(const everything_plugin_utf8_t *)"%s %s\r\n",command,param);

	// if there is a new command while we are currently querying, cancel the query.
	if (c->is_query)
	{
		// huh? we are not ready
		// the client needs to wait for the Query to complete.
//		etp_server_client_printf(c,(const everything_plugin_utf8_t *)"503 Invalid sequence of commands.\r\n");
		
		etp_server_client_printf(c,(const everything_plugin_utf8_t *)"426 Query aborted.\r\n");
		
		c->is_query = 0;
	}

	if (c->data_type != ETP_SERVER_CLIENT_DATA_TYPE_NONE)
	{
		// huh? we are not ready
		// the client needs to wait for the LIST or RETR to finish.
		etp_server_client_printf(c,(const everything_plugin_utf8_t *)"503 Invalid sequence of commands.\r\n");
	}
	else
	if (everything_plugin_utf8_string_compare_nocase_s_sla(command,(const everything_plugin_utf8_t *)"feat") == 0)
	{
		if (c->login_state == ETP_SERVER_CLIENT_LOGIN_STATE_LOGGED_IN)
		{
			//Response:	211-Features:
			// Response:	 MDTMx
			// Response:	 REST STREAM
			// Response:	 SIZE
			// Response:	 MLST type*;size*;modify*;
			// Response:	 MLSD
			// Response:	 UTF8
			// Response:	 CLNT
			// Response:	 MFMT
			// Response:	211 End
			etp_server_client_printf(c,(const everything_plugin_utf8_t *)"211-Features:\r\n"
				" MDTM\r\n"
				" REST STREAM\r\n"
				" SIZE\r\n"
				" MLST type*;size*;modify*;\r\n"
				" MLSD\r\n"
				" UTF8\r\n"
				" EVERYTHING\r\n"
				"211 End\r\n");
		}
		else
		{
			etp_server_client_printf(c,(const everything_plugin_utf8_t *)"530 Not logged on.\r\n");
		}
	}
	else	
	if (everything_plugin_utf8_string_compare_nocase_s_sla(command,(const everything_plugin_utf8_t *)"help") == 0)
	{
		if (c->login_state == ETP_SERVER_CLIENT_LOGIN_STATE_LOGGED_IN)
		{
			etp_server_client_printf(c,(const everything_plugin_utf8_t *)"214-Site commands:\r\n"
				" EVERYTHING\r\n"
				"214 End\r\n");
		}
		else
		{
			etp_server_client_printf(c,(const everything_plugin_utf8_t *)"530 Not logged on.\r\n");
		}
	}
	else	
	if (everything_plugin_utf8_string_compare_nocase_s_sla(command,(const everything_plugin_utf8_t *)"syst") == 0)
	{
		if (c->login_state == ETP_SERVER_CLIENT_LOGIN_STATE_LOGGED_IN)
		{
			etp_server_client_printf(c,(const everything_plugin_utf8_t *)"215 UNIX emulated by Everything\r\n");
		}
		else
		{
			etp_server_client_printf(c,(const everything_plugin_utf8_t *)"530 Not logged on.\r\n");
		}
	}
	else	
	if (everything_plugin_utf8_string_compare_nocase_s_sla(command,(const everything_plugin_utf8_t *)"cdup") == 0)
	{
		if (c->login_state == ETP_SERVER_CLIENT_LOGIN_STATE_LOGGED_IN)
		{
			etp_server_client_CDUP(c);
		}
		else
		{
			etp_server_client_printf(c,(const everything_plugin_utf8_t *)"530 Not logged on.\r\n");
		}
	}
	else	
	if (everything_plugin_utf8_string_compare_nocase_s_sla(command,(const everything_plugin_utf8_t *)"rest") == 0)
	{
		if (c->login_state == ETP_SERVER_CLIENT_LOGIN_STATE_LOGGED_IN)
		{
			c->rest = everything_plugin_utf8_string_to_qword(param);

			etp_server_client_printf(c,(const everything_plugin_utf8_t *)"350 Restarting at %I64u.\r\n",c->rest);
		}
		else
		{
			etp_server_client_printf(c,(const everything_plugin_utf8_t *)"530 Not logged on.\r\n");
		}
	}
	else	
	if (everything_plugin_utf8_string_compare_nocase_s_sla(command,(const everything_plugin_utf8_t *)"opts") == 0)
	{
		if (c->login_state == ETP_SERVER_CLIENT_LOGIN_STATE_LOGGED_IN)
		{
			if (everything_plugin_utf8_string_compare_nocase_s_sla(param,(const everything_plugin_utf8_t *)"utf8 on") == 0)
			{
				c->utf8_on = 1;
				etp_server_client_printf(c,(const everything_plugin_utf8_t *)"200 UTF8 mode enabled.\r\n");
			}
			else
			if (everything_plugin_utf8_string_compare_nocase_s_sla(param,(const everything_plugin_utf8_t *)"utf8 off") == 0)
			{
				c->utf8_on = 0;
				etp_server_client_printf(c,(const everything_plugin_utf8_t *)"200 UTF8 mode disabled.\r\n");
			}
			else
			{
				etp_server_client_printf(c,(const everything_plugin_utf8_t *)"501 Invalid option.\r\n");
			}
		}
		else
		{
			etp_server_client_printf(c,(const everything_plugin_utf8_t *)"530 Not logged on.\r\n");
		}
	}
	else
	if (everything_plugin_utf8_string_compare_nocase_s_sla(command,(const everything_plugin_utf8_t *)"port") == 0)
	{
		if (c->login_state == ETP_SERVER_CLIENT_LOGIN_STATE_LOGGED_IN)
		{
			if (etp_server_allow_ftp_port)
			{
				etp_server_client_PORT(c,param);
			}
			else
			{
				etp_server_client_printf(c,(const everything_plugin_utf8_t *)"501 Invalid option.\r\n",param);
			}
		}
		else
		{
			etp_server_client_printf(c,(const everything_plugin_utf8_t *)"530 Not logged on.\r\n");
		}
	}
	else
	if (everything_plugin_utf8_string_compare_nocase_s_sla(command,(const everything_plugin_utf8_t *)"quit") == 0)
	{
		etp_server_client_printf(c,(const everything_plugin_utf8_t *)"221 Goodbye.\r\n");
		
		c->is_quit = 1;
	}
	else
	if (everything_plugin_utf8_string_compare_nocase_s_sla(command,(const everything_plugin_utf8_t *)"mode") == 0)
	{
		if (c->login_state == ETP_SERVER_CLIENT_LOGIN_STATE_LOGGED_IN)
		{
			if ((everything_plugin_utf8_string_compare_nocase_s_sla(param,(const everything_plugin_utf8_t *)"s") == 0) || (everything_plugin_utf8_string_compare_nocase_s_sla(param,(const everything_plugin_utf8_t *)"b") == 0) || (everything_plugin_utf8_string_compare_nocase_s_sla(param,(const everything_plugin_utf8_t *)"c") == 0))
			{
				etp_server_client_printf(c,(const everything_plugin_utf8_t *)"504 Mode %s not implemented.\r\n",param);
			}
			else
			{
				etp_server_client_printf(c,(const everything_plugin_utf8_t *)"501 Invalid mode %s.\r\n",param);
			}
		}
		else
		{
			etp_server_client_printf(c,(const everything_plugin_utf8_t *)"530 Not logged on.\r\n");
		}
	}
	else
	if (everything_plugin_utf8_string_compare_nocase_s_sla(command,(const everything_plugin_utf8_t *)"stru") == 0)
	{
		if (c->login_state == ETP_SERVER_CLIENT_LOGIN_STATE_LOGGED_IN)
		{
			if ((everything_plugin_utf8_string_compare_nocase_s_sla(param,(const everything_plugin_utf8_t *)"f") == 0) || (everything_plugin_utf8_string_compare_nocase_s_sla(param,(const everything_plugin_utf8_t *)"r") == 0) || (everything_plugin_utf8_string_compare_nocase_s_sla(param,(const everything_plugin_utf8_t *)"p") == 0))
			{
				etp_server_client_printf(c,(const everything_plugin_utf8_t *)"504 File structure %s not implemented.\r\n",param);
			}
			else
			{
				etp_server_client_printf(c,(const everything_plugin_utf8_t *)"501 Invalid file structure %s.\r\n",param);
			}
		}
		else
		{
			etp_server_client_printf(c,(const everything_plugin_utf8_t *)"530 Not logged on.\r\n");
		}
	}
	else
	if (everything_plugin_utf8_string_compare_nocase_s_sla(command,(const everything_plugin_utf8_t *)"noop") == 0)
	{
		// ok
		etp_server_client_printf(c,(const everything_plugin_utf8_t *)"200 OK.\r\n");
	}
	else
	if (everything_plugin_utf8_string_compare_nocase_s_sla(command,(const everything_plugin_utf8_t *)"size") == 0)
	{
		if (c->login_state == ETP_SERVER_CLIENT_LOGIN_STATE_LOGGED_IN)
		{
			etp_server_SIZE(c,param);
		}
		else
		{
			etp_server_client_printf(c,(const everything_plugin_utf8_t *)"530 Not logged on.\r\n");
		}
	}
	else
	if (everything_plugin_utf8_string_compare_nocase_s_sla(command,(const everything_plugin_utf8_t *)"mdtm") == 0)
	{
		if (c->login_state == ETP_SERVER_CLIENT_LOGIN_STATE_LOGGED_IN)
		{
			// send list
			etp_server_client_MDTM(c,param);
		}
		else
		{
			etp_server_client_printf(c,(const everything_plugin_utf8_t *)"530 Not logged on.\r\n");
		}
	}
	else
	if (everything_plugin_utf8_string_compare_nocase_s_sla(command,(const everything_plugin_utf8_t *)"retr") == 0)
	{
		if (c->login_state == ETP_SERVER_CLIENT_LOGIN_STATE_LOGGED_IN)
		{
			etp_server_RETR(c,param);
		}
		else
		{
			etp_server_client_printf(c,(const everything_plugin_utf8_t *)"530 Not logged on.\r\n");
		}
	}
	else
	if (everything_plugin_utf8_string_compare_nocase_s_sla(command,(const everything_plugin_utf8_t *)"list") == 0)
	{
		if (c->login_state == ETP_SERVER_CLIENT_LOGIN_STATE_LOGGED_IN)
		{
			etp_server_client_LIST(c,param);
		}
		else
		{
			etp_server_client_printf(c,(const everything_plugin_utf8_t *)"530 Not logged on.\r\n");
		}
	}
	else
	if (everything_plugin_utf8_string_compare_nocase_s_sla(command,(const everything_plugin_utf8_t *)"mlsd") == 0)
	{
		if (c->login_state == ETP_SERVER_CLIENT_LOGIN_STATE_LOGGED_IN)
		{
			etp_server_client_MLSD(c,param);
		}
		else
		{
			etp_server_client_printf(c,(const everything_plugin_utf8_t *)"530 Not logged on.\r\n");
		}
	}
	else
	if (everything_plugin_utf8_string_compare_nocase_s_sla(command,(const everything_plugin_utf8_t *)"mlst") == 0)
	{
		if (c->login_state == ETP_SERVER_CLIENT_LOGIN_STATE_LOGGED_IN)
		{
			etp_server_client_MLST(c,param);
		}
		else
		{
			etp_server_client_printf(c,(const everything_plugin_utf8_t *)"530 Not logged on.\r\n");
		}
	}
	else
	if (everything_plugin_utf8_string_compare_nocase_s_sla(command,(const everything_plugin_utf8_t *)"pasv") == 0)
	{
		if (c->login_state == ETP_SERVER_CLIENT_LOGIN_STATE_LOGGED_IN)
		{
			etp_server_PASV(c);
		}
		else
		{
			etp_server_client_printf(c,(const everything_plugin_utf8_t *)"530 Not logged on.\r\n");
		}
	}
	else
	if (everything_plugin_utf8_string_compare_nocase_s_sla(command,(const everything_plugin_utf8_t *)"epsv") == 0)
	{
		if (c->login_state == ETP_SERVER_CLIENT_LOGIN_STATE_LOGGED_IN)
		{
			etp_server_client_EPSV(c);
		}
		else
		{
			etp_server_client_printf(c,(const everything_plugin_utf8_t *)"530 Not logged on.\r\n");
		}
	}
	else	
	if (everything_plugin_utf8_string_compare_nocase_s_sla(command,(const everything_plugin_utf8_t *)"eprt") == 0)
	{
		if (c->login_state == ETP_SERVER_CLIENT_LOGIN_STATE_LOGGED_IN)
		{
			if (etp_server_allow_ftp_port)
			{
				etp_server_client_EPRT(c,param);
			}
			else
			{
				etp_server_client_printf(c,(const everything_plugin_utf8_t *)"501 Invalid option.\r\n",param);
			}
		}
		else
		{
			etp_server_client_printf(c,(const everything_plugin_utf8_t *)"530 Not logged on.\r\n");
		}
	}
	else
	if (everything_plugin_utf8_string_compare_nocase_s_sla(command,(const everything_plugin_utf8_t *)"type") == 0)
	{
		if (c->login_state == ETP_SERVER_CLIENT_LOGIN_STATE_LOGGED_IN)
		{
			if (everything_plugin_utf8_string_compare_nocase_s_sla(param,(const everything_plugin_utf8_t *)"a") == 0)
			{
				etp_server_client_printf(c,(const everything_plugin_utf8_t *)"200 Type set to ASCII (A).\r\n");
			}
			else
			if (everything_plugin_utf8_string_compare_nocase_s_sla(param,(const everything_plugin_utf8_t *)"i") == 0)
			{
				etp_server_client_printf(c,(const everything_plugin_utf8_t *)"200 Type set to Image (I).\r\n");
			}
			else
			if (everything_plugin_utf8_string_compare_nocase_s_sla(param,(const everything_plugin_utf8_t *)"e") == 0)
			{
				etp_server_client_printf(c,(const everything_plugin_utf8_t *)"504 Type E not implemented.\r\n");
			}
			else
			{
				etp_server_client_printf(c,(const everything_plugin_utf8_t *)"500 Type %s not supported.\r\n",param);
			}
		}
		else
		{
			etp_server_client_printf(c,(const everything_plugin_utf8_t *)"530 Not logged on.\r\n");
		}
	}
	else
	if (everything_plugin_utf8_string_compare_nocase_s_sla(command,(const everything_plugin_utf8_t *)"cwd") == 0)
	{
		if (c->login_state == ETP_SERVER_CLIENT_LOGIN_STATE_LOGGED_IN)
		{
			etp_server_client_CWD(c,param);
		}
		else
		{
			etp_server_client_printf(c,(const everything_plugin_utf8_t *)"530 Not logged on.\r\n");
		}
	}
	else
	if ((everything_plugin_utf8_string_compare_nocase_s_sla(command,(const everything_plugin_utf8_t *)"pwd") == 0) || (everything_plugin_utf8_string_compare_nocase_s_sla(command,(const everything_plugin_utf8_t *)"xpwd") == 0))
	{
		// cmd ftp sends xpwd instead of pwd
		if (c->login_state == ETP_SERVER_CLIENT_LOGIN_STATE_LOGGED_IN)
		{
			everything_plugin_utf8_buf_t current_directory_cbuf;
			
			everything_plugin_utf8_buf_init(&current_directory_cbuf);

			etp_server_get_ftp_filename(c,&current_directory_cbuf,c->working_directory);

			etp_server_client_printf(c,(const everything_plugin_utf8_t *)"257 \"%s\" is current directory.\r\n",current_directory_cbuf.buf);

			everything_plugin_utf8_buf_kill(&current_directory_cbuf);
		}
		else
		{
			etp_server_client_printf(c,(const everything_plugin_utf8_t *)"530 Not logged on.\r\n");
		}
	}
	else
	if (everything_plugin_utf8_string_compare_nocase_s_sla(command,(const everything_plugin_utf8_t *)"everything") == 0)
	{
		if (c->login_state == ETP_SERVER_CLIENT_LOGIN_STATE_LOGGED_IN)
		{
			everything_plugin_utf8_t *p;
			
			p = param;
			
			while(*p)
			{
				if (everything_plugin_unicode_is_ascii_ws(*p))
				{
					*p++ = 0;
					
					break;
				}
				
				p++;
			}
			
			// dont skip over any more whitespaces
			// we want to process filenames starting with spaces.
			etp_server_client_everything(c,param,p);
		}
		else
		{
			etp_server_client_printf(c,(const everything_plugin_utf8_t *)"530 Not logged on.\r\n");
		}
	}
	else
	if (everything_plugin_utf8_string_compare_nocase_s_sla(command,(const everything_plugin_utf8_t *)"site") == 0)
	{
		if (c->login_state == ETP_SERVER_CLIENT_LOGIN_STATE_LOGGED_IN)
		{
			// find command and param parts
			{
				everything_plugin_utf8_t *p;
				
				command = param;
				p = param;
				
				while(*p)
				{
					if (everything_plugin_unicode_is_ascii_ws(*p)) 
					{
						*p++ = 0;

						break;
					}

					p++;
				}
				
				// dont skip any more white spaces
				// we want to be able to process filenames starting with spaces!
				param = p;
			}
			
			if (everything_plugin_utf8_string_compare_nocase_s_sla(command,(const everything_plugin_utf8_t *)"everything") == 0)
			{
				everything_plugin_utf8_t *p;
				
				p = param;
				
				while(*p)
				{
					if (everything_plugin_unicode_is_ascii_ws(*p))
					{
						*p++ = 0;
						
						break;
					}
					
					p++;
				}
				
				// dont skip over any more whitespaces
				// we want to process filenames starting with spaces.
				etp_server_client_everything(c,param,p);
			}
			else
			if (*command)
			{
				etp_server_client_printf(c,(const everything_plugin_utf8_t *)"500 Unknown SITE command.\r\n");
			}
			else
			{
				etp_server_client_printf(c,(const everything_plugin_utf8_t *)"500 SITE: Missing argument.\r\n");
			}
		}
		else
		{
			etp_server_client_printf(c,(const everything_plugin_utf8_t *)"530 Not logged on.\r\n");
		}
	}
	else
	if (everything_plugin_utf8_string_compare_nocase_s_sla(command,(const everything_plugin_utf8_t *)"user") == 0)
	{
		if ((*etp_server_username) && (everything_plugin_utf8_string_compare(etp_server_username,param) != 0))
		{
			// user is not valid.
			// request password so the user does not know if the username is invalid
			etp_server_client_printf(c,(const everything_plugin_utf8_t *)"331 Password required for %s\r\n",param);

			c->login_state = ETP_SERVER_CLIENT_LOGIN_STATE_INVALID_USER_NEED_PASSWORD;
		}
		else
		if (*etp_server_password)
		{
			// user is valid
			// need password
			etp_server_client_printf(c,(const everything_plugin_utf8_t *)"331 Password required for %s\r\n",param);

			c->login_state = ETP_SERVER_CLIENT_LOGIN_STATE_NEED_PASSWORD;
		}
		else
		{
			// user is valid.
			// no password needed, user logged on
			etp_server_client_printf(c,(const everything_plugin_utf8_t *)"230 Logged on.\r\n");

			c->login_state = ETP_SERVER_CLIENT_LOGIN_STATE_LOGGED_IN;
		}
	}
	else
	if (everything_plugin_utf8_string_compare_nocase_s_sla(command,(const everything_plugin_utf8_t *)"pass") == 0)
	{
		if (c->login_state == ETP_SERVER_CLIENT_LOGIN_STATE_NEED_PASSWORD)
		{
			// we will never get to this state if the length of the password is 0
			if (everything_plugin_utf8_string_compare(etp_server_password,param) == 0)
			{
				// password ok
				c->login_state = ETP_SERVER_CLIENT_LOGIN_STATE_LOGGED_IN;
				
				etp_server_client_printf(c,(const everything_plugin_utf8_t *)"230 Logged on.\r\n");
			}
			else
			{
				// invalid password
				etp_server_client_printf(c,(const everything_plugin_utf8_t *)"530 Login or password incorrect!\r\n");
			}
		}
		else
		if (c->login_state == ETP_SERVER_CLIENT_LOGIN_STATE_INVALID_USER_NEED_PASSWORD)
		{
			// invalid user
			etp_server_client_printf(c,(const everything_plugin_utf8_t *)"530 Login or password incorrect!\r\n");
		}
		else
		if (c->login_state == ETP_SERVER_CLIENT_LOGIN_STATE_LOGGED_IN)
		{
			// would have got 200 logged on before this
			etp_server_client_printf(c,(const everything_plugin_utf8_t *)"503 Invalid sequence of commands.\r\n");
		}
		else
		{
			etp_server_client_printf(c,(const everything_plugin_utf8_t *)"530 Not logged on.\r\n");
		}
	}
	else
	{
		etp_server_client_printf(c,(const everything_plugin_utf8_t *)"500 Syntax error, command unrecognized.\r\n");
	}
}

static int etp_server_client_update_recv(etp_server_client_t *c)
{
	for(;;)
	{
		int ret;
		
		// create a recv packet buffer..
		if (c->recv_front == c->recv_end)
		{
			etp_server_recv_chunk_add(c);
		}
		
		ret = everything_plugin_network_recv(c->control_socket,c->recv_front,(c->recv_end - c->recv_front));

everything_plugin_debug_printf("recv ret %d %d\n",ret,everything_plugin_os_winsock_WSAGetLastError());

		if (ret == EVERYTHING_PLUGIN_OS_WINSOCK_SOCKET_ERROR)
		{
			if (everything_plugin_os_winsock_WSAGetLastError() == WSAEWOULDBLOCK)
			{
				break;
			}
			
			return 0;
		}
		else
		if (ret == 0)
		{
			// socket closed.
			return 0;
		}
		else
		{
			everything_plugin_utf8_t *start;
			everything_plugin_utf8_t *p;
			int run;
			
			start = 0;
			p = (everything_plugin_utf8_t *)c->recv_front;
			run = ret;
			
			c->recv_front += ret;
			
			// did we read a newline?
			while(run)
			{
				if (*p == '\n')
				{
					*p = 0;
					
					if (start)
					{
						// another command in the same recv.
						etp_server_client_process_command(c,start);
					}
					else
					{
						if (c->recv_chunk_count == 1)
						{
							// a single packet.
							etp_server_client_process_command(c,ETP_SERVER_RECV_CHUNK_DATA(c->recv_chunk_last));
						}
						else
						{
							everything_plugin_utf8_t *linear_buf;
							everything_plugin_utf8_t *d;
							etp_server_recv_chunk_t *recv_chunk;

							// we MUST allocate a linear buffer
							linear_buf = everything_plugin_mem_alloc(((c->recv_chunk_count - 1) * (ETP_SERVER_RECV_CHUNK_SIZE - sizeof(etp_server_recv_chunk_t))) + (p - ETP_SERVER_RECV_CHUNK_DATA(c->recv_chunk_last)) + 1);
							
							d = linear_buf;
							
							// copy full chunks
							recv_chunk = c->recv_chunk_start;
							while(recv_chunk != c->recv_chunk_last)
							{
								etp_server_recv_chunk_t *next_recv_chunk;
								
								next_recv_chunk = recv_chunk->next;
								
								everything_plugin_os_copy_memory(d,recv_chunk+1,ETP_SERVER_RECV_CHUNK_SIZE - sizeof(etp_server_recv_chunk_t));
								
								d += ETP_SERVER_RECV_CHUNK_SIZE - sizeof(etp_server_recv_chunk_t);
								
								etp_server_recv_chunk_destroy(recv_chunk);
								
								recv_chunk = next_recv_chunk;
							}
							
							// last partial chunk
							everything_plugin_os_copy_memory(d,c->recv_chunk_last + 1,p - ETP_SERVER_RECV_CHUNK_DATA(c->recv_chunk_last));
							d += p - ETP_SERVER_RECV_CHUNK_DATA(c->recv_chunk_last);
							
							*d = 0;
							
							// keep last chunk.
							c->recv_chunk_count = 1;
							c->recv_chunk_start = c->recv_chunk_last;
							
							// execute command.
							etp_server_client_process_command(c,linear_buf);
							
							// free
							everything_plugin_mem_free(linear_buf);
						}
					}
					
					p++;
					start = p;
				}
				else
				{
					p++;
				}
				
				run--;
			}
			
			if (start)
			{
				// we processed at least one command
				// we must move any incomplete commands to the front of the buffer.
				everything_plugin_os_move_memory(c->recv_chunk_last + 1,start,c->recv_front - (char *)start);
				
				c->recv_front = (char *)(c->recv_chunk_last + 1) + (c->recv_front - (char *)start);
			}
		}
	}
	
	return 1;
}

static void etp_server_send_packet_destroy(etp_server_send_packet_t *send_packet)
{
	everything_plugin_mem_free(send_packet);
}

static int etp_server_client_update_send(etp_server_client_t *c)
{
	// send a packet
	for(;;)
	{
		int ret;
		
		// anything to send?
		if (!c->send_remaining) 
		{
			if (!c->send_start)
			{
				if (c->is_quit)
				{
					// we just sent goodbye in response to QUIT.
					everything_plugin_os_winsock_shutdown(c->control_socket,EVERYTHING_PLUGIN_OS_WINSOCK_SD_SEND);

					// close this control socket.
					return 0;
				}
			
				break;
			}
			
			c->send_remaining = c->send_start->size;
		}
		
everything_plugin_debug_printf("send %zu %t\n",c->send_remaining,((char *)(c->send_start+1)) + c->send_start->size - c->send_remaining,c->send_remaining);

		// c->send_cur->packet_len would be nonzero
		// we test for this after sending the header.
		// send data
		ret = everything_plugin_network_send(c->control_socket,((char *)(c->send_start+1)) + c->send_start->size - c->send_remaining,c->send_remaining);
		
everything_plugin_debug_printf("send ret %d\n",ret);

		if (ret == EVERYTHING_PLUGIN_OS_WINSOCK_SOCKET_ERROR)
		{
			if (everything_plugin_os_winsock_WSAGetLastError() == WSAEWOULDBLOCK)
			{
				break;
			}

everything_plugin_debug_printf("send error %d\n",everything_plugin_os_winsock_WSAGetLastError());
			
			// socket error.
			return 0;
		}
		else
		if (ret == 0)
		{
everything_plugin_debug_printf("send closed\n");
			// socket closed.
			return 0;
		}
		else
		{
			c->send_remaining -= ret;
			
			// did we send everything?
			if (!c->send_remaining)
			{
				etp_server_send_packet_t *next_send_packet;
				
				// destroy the first packet
				next_send_packet = c->send_start->next;
				
				etp_server_send_packet_destroy(c->send_start);
				
				c->send_start = next_send_packet;
				if (!c->send_start)
				{
					c->send_last = 0;
				}
			}
		}
	}
		
	return 1;
}

static int etp_server_client_update_data(etp_server_client_t *c)
{
	// send list
	if ((c->data_type == ETP_SERVER_CLIENT_DATA_TYPE_LIST) || (c->data_type == ETP_SERVER_CLIENT_DATA_TYPE_MLSD))
	{
		for(;;)
		{
			int ret;
			
			// anything to send?
			if (!c->data.list.remaining) 
			{
				if (!c->data.list.chunk_start)
				{
					c->data_complete = 1;
					
					return 0;
				}
				
				c->data.list.remaining = c->data.list.chunk_start->size;
			}
			
			// c->send_cur->packet_len would be nonzero
			// we test for this after sending the header.
			// send data
			ret = everything_plugin_network_send(c->data_socket,((char *)(c->data.list.chunk_start+1)) + c->data.list.chunk_start->size - c->data.list.remaining,c->data.list.remaining);

			if (ret == EVERYTHING_PLUGIN_OS_WINSOCK_SOCKET_ERROR)
			{
				if (everything_plugin_os_winsock_WSAGetLastError() == WSAEWOULDBLOCK)
				{
					break;
				}
				
				// socket error.
				etp_server_client_printf(c,(const everything_plugin_utf8_t *)"500 send failed %d.\r\n",everything_plugin_os_winsock_WSAGetLastError());

				return 0;
			}
			else
			if (ret == 0)
			{
				// socket closed.
				etp_server_client_printf(c,(const everything_plugin_utf8_t *)"426 Connection closed; transfer aborted.\r\n");
				
				return 0;
			}
			else
			{
				c->data.list.remaining -= ret;
				
				// destroy start chunk
				if (!c->data.list.remaining)
				{
					etp_server_list_chunk_t *next_list_chunk;
					
					next_list_chunk = c->data.list.chunk_start->next;
					
					everything_plugin_mem_free(c->data.list.chunk_start);
					
					c->data.list.chunk_start = next_list_chunk;
					if (!c->data.list.chunk_start)
					{
						c->data.list.chunk_last = 0;
					}
				}
			}
		}
	}
	
	// update RETR
	if (c->data_type == ETP_SERVER_CLIENT_DATA_TYPE_RETR)
	{
		for(;;)
		{
			int ret;
			uintptr_t retr_remaining;
			uintptr_t retr_size;
			int retr_state;
			
			// fill buffer.
			EnterCriticalSection(&c->data.retr.cs);
			
			retr_remaining = c->data.retr.remaining;
			retr_size = c->data.retr.size;
			retr_state = c->data.retr.state;
			
			LeaveCriticalSection(&c->data.retr.cs);
			
			if (!retr_remaining)
			{
				// read
				if (retr_state == 1)
				{
					c->data_complete = 1;
					
					return 0;
				}
				else
				if (retr_state == 2)
				{
					// finished
					etp_server_client_printf(c,(const everything_plugin_utf8_t *)"451 Transfer aborted: read error.\r\n");
					
					return 0;
				}
				else
				if (retr_state == 0)
				{
					// request more data..
					SetEvent(c->data.retr.hevent);
					
					break;
				}
				
				break;
			}

			// c->send_cur->packet_len would be nonzero
			// we test for this after sending the header.
			// send data
			ret = everything_plugin_network_send(c->data_socket,c->data.retr.buffer + retr_size - retr_remaining,retr_remaining);

			if (ret == EVERYTHING_PLUGIN_OS_WINSOCK_SOCKET_ERROR)
			{
				if (everything_plugin_os_winsock_WSAGetLastError() == WSAEWOULDBLOCK)
				{
					break;
				}
				
				// socket error.
				etp_server_client_printf(c,(const everything_plugin_utf8_t *)"426 send failed %d\r\n",everything_plugin_os_winsock_WSAGetLastError());
				
				return 0;
			}
			else
			if (ret == 0)
			{
				// socket closed.
				etp_server_client_printf(c,(const everything_plugin_utf8_t *)"426 Connection closed; transfer aborted.\r\n");
				
				return 0;
			}
			else
			{
				EnterCriticalSection(&c->data.retr.cs);
				c->data.retr.remaining = retr_remaining - ret;
				LeaveCriticalSection(&c->data.retr.cs);
			}
		}
	}
		
	return 1;
}

// size MUST be > 0
static void etp_server_send_packet_add(etp_server_client_t *c,void *data,uintptr_t size)
{
	// try sending now.
	etp_server_send_packet_t *send_packet;
	
	if (everything_plugin_debug_is_verbose())
	{
		everything_plugin_debug_color_printf(0xff00ff00,(const everything_plugin_utf8_t *)"%t",data,size);
	}
	
	// alloc
	send_packet = everything_plugin_mem_alloc(sizeof(etp_server_send_packet_t) + size);
	
	// init
	send_packet->size = size;
	everything_plugin_os_copy_memory(send_packet + 1,data,size);
	
	// insert
	if (c->send_start)
	{
		c->send_last->next = send_packet;
	}
	else
	{
		c->send_start = send_packet;
		
		// set up the first packet to send
		c->send_remaining = send_packet->size;
		
		// update send
		PostMessage(_etp_server->hwnd,ETP_SERVER_WM_CLIENT,c->control_socket,0);
	}
	
	send_packet->next = 0;
	c->send_last = send_packet;
}

static void etp_server_recv_chunk_add(etp_server_client_t *c)
{
	etp_server_recv_chunk_t *recv_chunk;	
	
	recv_chunk = everything_plugin_mem_alloc(ETP_SERVER_RECV_CHUNK_SIZE);
	
	if (c->recv_chunk_start)
	{
		c->recv_chunk_last->next = recv_chunk;
	}
	else
	{
		c->recv_chunk_start = recv_chunk;
	}
	
	recv_chunk->next = 0;
	c->recv_chunk_last = recv_chunk;
	c->recv_chunk_count++;
	
	c->recv_end = ((char *)recv_chunk) + ETP_SERVER_RECV_CHUNK_SIZE;
	c->recv_front = (char *)(recv_chunk + 1);
}	


// get the localhost everything_plugin_os_winsock_addrinfo
// save stack from main too.
static int etp_server_get_bind_addrinfo(const everything_plugin_utf8_t *nodename,struct everything_plugin_os_winsock_addrinfo **ai)
{
	struct everything_plugin_os_winsock_addrinfo hints;
	everything_plugin_utf8_buf_t port_cbuf;
	int ret;
	
	everything_plugin_utf8_buf_init(&port_cbuf);
	ret = 0;

	// Fill out the local socket address data.
	everything_plugin_os_zero_memory(&hints,sizeof(struct everything_plugin_os_winsock_addrinfo));
	hints.ai_protocol = EVERYTHING_PLUGIN_OS_WINSOCK_IPPROTO_TCP;
	hints.ai_socktype = EVERYTHING_PLUGIN_OS_WINSOCK_SOCK_STREAM;
	hints.ai_flags = EVERYTHING_PLUGIN_OS_WINSOCK_AI_PASSIVE;	

	everything_plugin_utf8_buf_printf(&port_cbuf,(const everything_plugin_utf8_t *)"%d",_etp_server->port);

	if (everything_plugin_os_winsock_getaddrinfo((const char *)nodename,(const char *)port_cbuf.buf,&hints,ai) == 0)
	{
		ret = 1;
	}

	everything_plugin_utf8_buf_kill(&port_cbuf);
	
	return ret;
}


static int etp_server_add_binding(everything_plugin_utf8_buf_t *error_cbuf,const everything_plugin_utf8_t *nodename)
{
	struct everything_plugin_os_winsock_addrinfo *ai;
	int ret;
	
	ret = 0;
		
	if (etp_server_get_bind_addrinfo(nodename,&ai))
	{
		struct everything_plugin_os_winsock_addrinfo *aip;
		
		aip = ai;
		
		while(aip)
		{
			// ipv4 or ipv6 please.
			if ((aip->ai_family == EVERYTHING_PLUGIN_OS_WINSOCK_AF_INET) || (aip->ai_family == EVERYTHING_PLUGIN_OS_WINSOCK_AF_INET6))
			{
				EVERYTHING_PLUGIN_OS_WINSOCK_SOCKET listen_socket;
				
				// reset ret, as a previous bind would have set it to 1.
				ret = 0;
				
	//DBEUG:
	everything_plugin_debug_printf((const everything_plugin_utf8_t *)"bind to family %d, protocol %d, socktype %d\n",aip->ai_family,aip->ai_protocol,aip->ai_socktype);

				listen_socket = everything_plugin_os_winsock_socket(aip->ai_family,aip->ai_socktype,aip->ai_protocol);
				if (listen_socket != EVERYTHING_PLUGIN_OS_WINSOCK_INVALID_SOCKET)
				{
					if (everything_plugin_os_winsock_bind(listen_socket,aip->ai_addr,(int)aip->ai_addrlen) != EVERYTHING_PLUGIN_OS_WINSOCK_SOCKET_ERROR)
					{
						if (everything_plugin_os_winsock_listen(listen_socket,EVERYTHING_PLUGIN_OS_WINSOCK_SOMAXCONN) != EVERYTHING_PLUGIN_OS_WINSOCK_SOCKET_ERROR)
						{
							etp_server_listen_t *listen;
							
							// alloc
							listen = everything_plugin_mem_alloc(sizeof(etp_server_listen_t));
							
							// init
							listen->listen_socket = listen_socket;
							
							// insert
							if (_etp_server->listen_start)
							{
								_etp_server->listen_last->next = listen;
							}
							else
							{
								_etp_server->listen_start = listen;
							}
							
							listen->next = 0;
							_etp_server->listen_last = listen;
							
							// select
							everything_plugin_os_winsock_WSAAsyncSelect(listen_socket,_etp_server->hwnd,ETP_SERVER_WM_LISTEN,EVERYTHING_PLUGIN_OS_WINSOCK_FD_ACCEPT|EVERYTHING_PLUGIN_OS_WINSOCK_FD_CLOSE);
							
							ret = 1;

							goto next_ai;
						}
						else
						{
							everything_plugin_utf8_buf_printf(error_cbuf,everything_plugin_localization_get_string(EVERYTHING_PLUGIN_LOCALIZATION_LISTEN_FAILED_FORMAT),everything_plugin_os_winsock_WSAGetLastError());
						}
					}
					else
					{
						everything_plugin_debug_error_printf("bind failed %d\n",everything_plugin_os_winsock_WSAGetLastError());
					
						everything_plugin_utf8_buf_printf(error_cbuf,everything_plugin_localization_get_string(EVERYTHING_PLUGIN_LOCALIZATION_BIND_FAILED_FORMAT),everything_plugin_os_winsock_WSAGetLastError());
					}

					everything_plugin_os_winsock_closesocket(listen_socket);
				}
				else
				{
					everything_plugin_utf8_buf_printf(error_cbuf,everything_plugin_localization_get_string(EVERYTHING_PLUGIN_LOCALIZATION_SOCKET_FAILED_FORMAT),everything_plugin_os_winsock_WSAGetLastError());
				}

				if (!ret)
				{
					break;
				}
			}			
			
next_ai:

			
			aip = aip->ai_next;
		}
		
		everything_plugin_os_winsock_freeaddrinfo(ai);
	}
	
	return ret;
}

static void etp_server_start(void)
{
	everything_plugin_utf8_buf_t error_cbuf;
	
	everything_plugin_utf8_buf_init(&error_cbuf);
	
	if (!_etp_server)
	{
		int wsaret;
		EVERYTHING_PLUGIN_OS_WINSOCK_WSADATA wsadata;

		// please use version 1.1 to be compatible with other plugins.
		wsaret = everything_plugin_os_winsock_WSAStartup(MAKEWORD(1,1),&wsadata);
		if (wsaret == 0)
		{
			// make server now, as it will hold the ref to WSAStartup.
			_etp_server = everything_plugin_mem_calloc(sizeof(etp_server_t));
			
			_etp_server->db = everything_plugin_db_add_local_ref();
			_etp_server->bindings = everything_plugin_utf8_string_alloc_utf8_string(etp_server_bindings);
			_etp_server->port = etp_server_port;
			
			everything_plugin_os_register_class(0,(const everything_plugin_utf8_t *)"EVERYTHING_ETP_SERVER",etp_server_proc,0,0,0,0);

			_etp_server->hwnd = everything_plugin_os_create_window(
				0,
				(const everything_plugin_utf8_t *)"EVERYTHING_ETP_SERVER",
				(const everything_plugin_utf8_t *)"EVERYTHING_ETP_SERVER",
				0,0,0,0,0,0,0,GetModuleHandle(0),0);

			// make sure we got winsock 1.1 
			if ((LOBYTE(wsadata.wVersion) == 1) || (HIBYTE(wsadata.wVersion) == 1))
			{
				// parse the list of bindings.
				if ((*etp_server_bindings) && (everything_plugin_utf8_string_compare(etp_server_bindings,"*") != 0))
				{
					const everything_plugin_utf8_t *bindp;
					everything_plugin_utf8_buf_t bind_cbuf;
					
					everything_plugin_utf8_buf_init(&bind_cbuf);
					
					bindp = etp_server_bindings;
				
					for(;;)
					{
						bindp = everything_plugin_utf8_string_parse_csv_item(bindp,&bind_cbuf);
						if (!bindp)
						{
							break;
						}
						
						if (*bind_cbuf.buf)
						{
							if (!etp_server_add_binding(&error_cbuf,bind_cbuf.buf))
							{
								break;
							}
						}
					}

					everything_plugin_utf8_buf_kill(&bind_cbuf);
				}
				else
				{
					etp_server_add_binding(&error_cbuf,0);
				}
			}
			else
			{
				everything_plugin_utf8_buf_printf(&error_cbuf,everything_plugin_localization_get_string(EVERYTHING_PLUGIN_LOCALIZATION_UNSUPPORTED_WSADATA_FORMAT),LOBYTE(wsadata.wVersion),HIBYTE(wsadata.wVersion));
			}
						
			// did we bind to anything?
			if (_etp_server->listen_start)
			{
				if (*error_cbuf.buf)
				{
					everything_plugin_debug_error_printf("ETP server started, but with errors: %s\n",error_cbuf.buf);
				}
			
				etp_server_log(0,(const everything_plugin_utf8_t *)"ETP server online.\r\n");
	
				goto exit;
			}

			etp_server_shutdown();
		}
		else
		{
			everything_plugin_utf8_buf_printf(&error_cbuf,everything_plugin_localization_get_string(EVERYTHING_PLUGIN_LOCALIZATION_WSASTARTUP_FAILED_FORMAT),wsaret);
		}
	}

	if (*error_cbuf.buf)
	{
		everything_plugin_ui_task_dialog_show(NULL,MB_OK|MB_ICONERROR,everything_plugin_localization_get_string(EVERYTHING_PLUGIN_LOCALIZATION_EVERYTHING),NULL,everything_plugin_localization_get_string(EVERYTHING_PLUGIN_LOCALIZATION_ETP_SERVER_ERROR_FORMAT),error_cbuf.buf);
	}

exit:

	everything_plugin_utf8_buf_kill(&error_cbuf);
}

static void EVERYTHING_PLUGIN_API etp_server_db_query_event_proc(etp_server_client_t *c,int type)
{
	switch(type)
	{
		case EVERYTHING_PLUGIN_DB_QUERY_EVENT_QUERY_COMPLETE:
		case EVERYTHING_PLUGIN_DB_QUERY_EVENT_SORT_COMPLETE:
		
			if (c->is_query)
			{
				etp_server_send_query_results(c);
				
				c->is_query = 0;
			}
			
			break;
	}
}

// log
static void etp_server_log(etp_server_client_t *c,const everything_plugin_utf8_t *format,...)
{
	if (etp_server_logging_enabled)
	{
		everything_plugin_utf8_buf_t message_cbuf;
		va_list argptr;
		
		everything_plugin_utf8_buf_init(&message_cbuf);
		
		va_start(argptr,format);

		everything_plugin_utf8_buf_vprintf(&message_cbuf,format,argptr);
			
		va_end(argptr);
		
		everything_plugin_debug_color_printf(0xff00ffff,(const everything_plugin_utf8_t *)"%s",message_cbuf.buf);
		
		if (!_etp_server->log_file)
		{
			everything_plugin_utf8_buf_t filename_cbuf;
			
			everything_plugin_utf8_buf_init(&filename_cbuf);
		
			if (*etp_server_log_file_name)
			{
				everything_plugin_os_get_local_app_data_path_cat_filename(etp_server_log_file_name,&filename_cbuf);
			}
			else
			{
				everything_plugin_os_get_local_app_data_path_cat_make_filename((const everything_plugin_utf8_t *)"Logs\\ETP_Server_Log",(const everything_plugin_utf8_t *)".txt",&filename_cbuf);
			}
			
			everything_plugin_os_resize_file(filename_cbuf.buf,etp_server_log_max_size,etp_server_log_delta_size);

			everything_plugin_os_make_sure_path_to_file_exists(filename_cbuf.buf);

			_etp_server->log_file = everything_plugin_output_stream_append_file(filename_cbuf.buf);

			everything_plugin_utf8_buf_kill(&filename_cbuf);
		}
		
	
		if (_etp_server->log_file)
		{
			EVERYTHING_PLUGIN_QWORD ft;
			
			// write date time
			ft = everything_plugin_os_get_system_time_as_file_time();

			{
				everything_plugin_utf8_buf_t date_time_cbuf;
				everything_plugin_utf8_buf_t version_cbuf;
				
				everything_plugin_utf8_buf_init(&version_cbuf);
				everything_plugin_utf8_buf_init(&date_time_cbuf);
			
				// get version	
				everything_plugin_version_get_text(&version_cbuf);

				// init_format_system_time?
				everything_plugin_utf8_buf_format_filetime(&date_time_cbuf,ft);
				
				everything_plugin_output_stream_write_printf(_etp_server->log_file,(const everything_plugin_utf8_t *)"%s: ",date_time_cbuf.buf);
				
				// write to file.
				if (c)
				{
					everything_plugin_utf8_buf_t sockaddr_cbuf;
					everything_plugin_utf8_buf_init(&sockaddr_cbuf);

					everything_plugin_utf8_buf_format_peername(&sockaddr_cbuf,c->control_socket);
					
					everything_plugin_output_stream_write_printf(_etp_server->log_file,(const everything_plugin_utf8_t *)"%p: ",c->control_socket);
					
					everything_plugin_output_stream_write_printf(_etp_server->log_file,(const everything_plugin_utf8_t *)"%s: ",sockaddr_cbuf.buf);

					everything_plugin_utf8_buf_kill(&sockaddr_cbuf);
				}
				
				everything_plugin_output_stream_write_printf(_etp_server->log_file,(const everything_plugin_utf8_t *)"%s",message_cbuf.buf);

				everything_plugin_utf8_buf_kill(&date_time_cbuf);
				everything_plugin_utf8_buf_kill(&version_cbuf);
			}
		}

		everything_plugin_utf8_buf_kill(&message_cbuf);
	}
}

static void etp_server_client_printf(etp_server_client_t *c,const everything_plugin_utf8_t *format,...)
{
	everything_plugin_utf8_buf_t	cbuf;
	
	everything_plugin_utf8_buf_init(&cbuf);

	{
		va_list argptr;
			
		va_start(argptr,format);
	
		everything_plugin_utf8_buf_vprintf(&cbuf,format,argptr);

		va_end(argptr);
	}

	etp_server_send_packet_add(c,cbuf.buf,cbuf.len);
	
	if ((*cbuf.buf != ' ') && (cbuf.buf[0]) && (cbuf.buf[1]) && (cbuf.buf[2]) && (cbuf.buf[3] == ' '))
	{
		etp_server_log(c,(const everything_plugin_utf8_t *)"%s",cbuf.buf);
	}

	everything_plugin_utf8_buf_kill(&cbuf);
}

static void etp_server_recv_chunk_destroy(etp_server_recv_chunk_t *recv_chunk)
{
	everything_plugin_mem_free(recv_chunk);
}

// pasv mode
static void etp_server_client_EPSV(etp_server_client_t *c)
{
	int family;
	
	// close existing data connection
	etp_server_close_data(c);
	
	// use the same family as the control socket.
	family = c->is_ipv6 ? EVERYTHING_PLUGIN_OS_WINSOCK_PF_INET6 : EVERYTHING_PLUGIN_OS_WINSOCK_PF_INET;

    // create the _etp_server socket
	c->data_connection_data.pasv.socket_handle = everything_plugin_os_winsock_socket(family,EVERYTHING_PLUGIN_OS_WINSOCK_SOCK_STREAM,EVERYTHING_PLUGIN_OS_WINSOCK_IPPROTO_TCP);
	if (c->data_connection_data.pasv.socket_handle != EVERYTHING_PLUGIN_OS_WINSOCK_INVALID_SOCKET) 
	{
		int bindret;
		
		everything_plugin_network_set_tcp_nodelay(c->data_connection_data.pasv.socket_handle);
		everything_plugin_network_set_keepalive(c->data_connection_data.pasv.socket_handle);
		
		everything_plugin_os_winsock_WSAAsyncSelect(c->data_connection_data.pasv.socket_handle,_etp_server->hwnd,ETP_SERVER_WM_PASV,EVERYTHING_PLUGIN_OS_WINSOCK_FD_ACCEPT|EVERYTHING_PLUGIN_OS_WINSOCK_FD_CLOSE);
		
		if (family == EVERYTHING_PLUGIN_OS_WINSOCK_PF_INET6)
		{
			struct everything_plugin_os_winsock_sockaddr_in6 data_sockaddr;
			
			everything_plugin_os_zero_memory(&data_sockaddr,sizeof(struct everything_plugin_os_winsock_sockaddr_in6));
			
			data_sockaddr.sin6_family = EVERYTHING_PLUGIN_OS_WINSOCK_AF_INET6;
			
			bindret = everything_plugin_os_winsock_bind(c->data_connection_data.pasv.socket_handle,(struct everything_plugin_os_winsock_sockaddr *)&data_sockaddr,sizeof(struct everything_plugin_os_winsock_sockaddr_in6));
		}
		else
		{
			struct everything_plugin_os_sockaddr_in data_sockaddr;
			
			everything_plugin_os_zero_memory(&data_sockaddr,sizeof(struct everything_plugin_os_sockaddr_in));
			
			data_sockaddr.sin_family = EVERYTHING_PLUGIN_OS_WINSOCK_AF_INET;
			
			bindret = everything_plugin_os_winsock_bind(c->data_connection_data.pasv.socket_handle,(struct everything_plugin_os_winsock_sockaddr *)&data_sockaddr,sizeof(struct everything_plugin_os_sockaddr_in));
		}

		// Associate the local address with WinSocket.
		if (bindret != EVERYTHING_PLUGIN_OS_WINSOCK_SOCKET_ERROR) 
		{
			// listen for incoming connections	
			if (everything_plugin_os_winsock_listen(c->data_connection_data.pasv.socket_handle,EVERYTHING_PLUGIN_OS_WINSOCK_SOMAXCONN) != EVERYTHING_PLUGIN_OS_WINSOCK_SOCKET_ERROR)
			{
				int port;
				struct everything_plugin_os_winsock_sockaddr_storage control_sockaddr;
				int sockaddr_size;
					
				sockaddr_size = sizeof(struct everything_plugin_os_winsock_sockaddr_storage);
				
				if (everything_plugin_os_winsock_getsockname(c->data_connection_data.pasv.socket_handle,(struct everything_plugin_os_winsock_sockaddr *)&control_sockaddr,&sockaddr_size) != EVERYTHING_PLUGIN_OS_WINSOCK_SOCKET_ERROR)
				{
					if (control_sockaddr.ss_family == EVERYTHING_PLUGIN_OS_WINSOCK_PF_INET6)
					{
						port = ((struct everything_plugin_os_winsock_sockaddr_in6 *)&control_sockaddr)->sin6_port;
					}
					else
					if (control_sockaddr.ss_family == EVERYTHING_PLUGIN_OS_WINSOCK_PF_INET)
					{
						port = ((struct everything_plugin_os_sockaddr_in *)&control_sockaddr)->sin_port;
					}
					else
					{
						// unsupported family.
						port = 0;
					}
					
					if (port)
					{
						etp_server_client_printf(c,(const everything_plugin_utf8_t *)"229 Entering Extended Passive Mode (|||%d|)\r\n",everything_plugin_os_winsock_ntohs(port));
						
						c->data_connection_type = ETP_SERVER_CLIENT_DATA_CONNECTION_TYPE_PASV;
						c->data_is_connected = 1;
					}
					else
					{
						etp_server_close_data(c);
						
						etp_server_client_printf(c,(const everything_plugin_utf8_t *)"500 Failed to enter passive mode, invalid family %d\r\n",control_sockaddr.ss_family);
					}
				}
				else
				{
					etp_server_close_data(c);
					
					etp_server_client_printf(c,(const everything_plugin_utf8_t *)"500 Failed to enter passive mode, getsockname failed %d\r\n",everything_plugin_os_winsock_WSAGetLastError());
				}
			}
			else
			{
				etp_server_close_data(c);
				
				etp_server_client_printf(c,(const everything_plugin_utf8_t *)"500 Failed to enter passive mode, socket listen %d\r\n",everything_plugin_os_winsock_WSAGetLastError());
			}
		}
		else
		{
			etp_server_close_data(c);
			
			etp_server_client_printf(c,(const everything_plugin_utf8_t *)"500 Failed to enter passive mode, bind failed %d\r\n",everything_plugin_os_winsock_WSAGetLastError());
		}
	}
	else
	{
		etp_server_close_data(c);
		
		etp_server_client_printf(c,(const everything_plugin_utf8_t *)"500 Failed to enter passive mode, socket failed %d\r\n",everything_plugin_os_winsock_WSAGetLastError());
	}
}

// port
static void etp_server_client_EPRT(etp_server_client_t *c,const everything_plugin_utf8_t *address)
{
	int family;
	int port;
	const everything_plugin_utf8_t *p;
	int argi;
	everything_plugin_utf8_buf_t host_cbuf;
	
	everything_plugin_utf8_buf_init(&host_cbuf);
	p = address;
	port = 0;
	family = EVERYTHING_PLUGIN_OS_WINSOCK_AF_UNSPEC;
		
	for(argi=0;argi<4;argi++)
	{
		const everything_plugin_utf8_t *start;
		
		start = p;
		
		while(*p)
		{
			if (*p == '|') 
			{
				break;
			}
			
			p++;
		}
		
		switch(argi)
		{
			case 1:
				switch(everything_plugin_utf8_string_to_int(start))
				{
					case 1: family = EVERYTHING_PLUGIN_OS_WINSOCK_AF_INET; break;
					case 2: family = EVERYTHING_PLUGIN_OS_WINSOCK_AF_INET6; break;
				}
				break;

			case 2:
				everything_plugin_utf8_buf_copy_utf8_string_n(&host_cbuf,start,p - start);
				break;

			case 3:
				port = _byteswap_ushort(everything_plugin_utf8_string_to_int(start));
				break;
		}
		
		if (*p)
		{
			// skip the '|'
			p++;
		}
	}
	
	// close existing data connection.
	etp_server_close_data(c);

    // create the _etp_server socket
    if (family != EVERYTHING_PLUGIN_OS_WINSOCK_AF_UNSPEC)
    {
		// port MUST match client ip
		if (etp_server_check_ftp_data_connection_ip)
		{
			if (family == EVERYTHING_PLUGIN_OS_WINSOCK_AF_INET)
			{
				everything_plugin_utf8_string_parse_sockaddr_in(host_cbuf.buf,(struct everything_plugin_os_sockaddr_in *)&c->data_connection_data.eprt_addr);
				((struct everything_plugin_os_sockaddr_in *)&c->data_connection_data.eprt_addr)->sin_port = port;
			}
			else
			{
				everything_plugin_utf8_string_parse_sockaddr_in6(host_cbuf.buf,(struct everything_plugin_os_winsock_sockaddr_in6 *)&c->data_connection_data.eprt_addr);
				((struct everything_plugin_os_winsock_sockaddr_in6 *)&c->data_connection_data.eprt_addr)->sin6_port = port;
			}
			/*
			{
				everything_plugin_utf8_buf_t host_buf;
				
				everything_plugin_utf8_buf_init(&host_buf);

				if (c->data_connection_data.eprt_addr.ss_family == OS_WINSOCK_AF_INET)
				{
					everything_plugin_utf8_buf_format_sockaddr_in(&host_buf,(struct os_sockaddr_in *)&c->data_connection_data.eprt_addr);
				}
				else
				{
					everything_plugin_utf8_buf_format_struct everything_plugin_os_winsock_sockaddr_in6(&host_buf,(struct everything_plugin_os_winsock_sockaddr_in6 *)&c->data_connection_data.eprt_addr);
				}

				etp_server_client_printf(c,(const everything_plugin_utf8_t *)" eprt_addr %s.\r\n",host_buf.buf);

				if (c->control_addr.ss_family == OS_WINSOCK_AF_INET)
				{
					everything_plugin_utf8_buf_format_sockaddr_in(&host_buf,(struct os_sockaddr_in *)&c->control_addr);
				}
				else
				{
					everything_plugin_utf8_buf_format_struct everything_plugin_os_winsock_sockaddr_in6(&host_buf,(struct everything_plugin_os_winsock_sockaddr_in6 *)&c->control_addr);
				}

				etp_server_client_printf(c,(const everything_plugin_utf8_t *)" control_addr %s.\r\n",host_buf.buf);

				everything_plugin_utf8_buf_kill(&host_buf);
			}*/

			if (!etp_server_addr_is_host_equal((struct everything_plugin_os_winsock_sockaddr *)&c->control_addr,(struct everything_plugin_os_winsock_sockaddr *)&c->data_connection_data.eprt_addr))
			{
				etp_server_client_printf(c,(const everything_plugin_utf8_t *)"500 Failed to connect to IP.\r\n");
				
				goto bad_ip;
			}
		}
		
		c->data_connection_type = ETP_SERVER_CLIENT_DATA_CONNECTION_TYPE_EPRT;
		c->data_is_connected = 0;

		etp_server_client_printf(c,(const everything_plugin_utf8_t *)"200 Port command successful.\r\n");
	}
	else
	{
		etp_server_client_printf(c,(const everything_plugin_utf8_t *)"522 Extended Port Failure - unknown network protocol. Supported protocols: (1,2)\r\n");

		etp_server_close_data(c);
	}
	
bad_ip:	
	
	everything_plugin_utf8_buf_kill(&host_cbuf);
}

static int etp_server_addr_is_host_equal(struct everything_plugin_os_winsock_sockaddr *a,struct everything_plugin_os_winsock_sockaddr *b)
{
	if (a->sa_family == b->sa_family)
	{
		if (a->sa_family == EVERYTHING_PLUGIN_OS_WINSOCK_AF_INET)
		{
			if ((((struct everything_plugin_os_sockaddr_in *)a)->sin_addr.S_un.S_un_b.s_b1 == ((struct everything_plugin_os_sockaddr_in *)b)->sin_addr.S_un.S_un_b.s_b1) && (((struct everything_plugin_os_sockaddr_in *)a)->sin_addr.S_un.S_un_b.s_b2 == ((struct everything_plugin_os_sockaddr_in *)b)->sin_addr.S_un.S_un_b.s_b2) && (((struct everything_plugin_os_sockaddr_in *)a)->sin_addr.S_un.S_un_b.s_b3 == ((struct everything_plugin_os_sockaddr_in *)b)->sin_addr.S_un.S_un_b.s_b3) && (((struct everything_plugin_os_sockaddr_in *)a)->sin_addr.S_un.S_un_b.s_b4 == ((struct everything_plugin_os_sockaddr_in *)b)->sin_addr.S_un.S_un_b.s_b4))
			{
				return 1;
			}
		}

		if (a->sa_family == EVERYTHING_PLUGIN_OS_WINSOCK_AF_INET6)
		{
			int addri;
			
			for(addri=0;addri<16;addri++)
			{
				if (((struct everything_plugin_os_winsock_sockaddr_in6 *)a)->sin6_addr.u.Byte[addri] != ((struct everything_plugin_os_winsock_sockaddr_in6 *)b)->sin6_addr.u.Byte[addri])
				{
					return 0;
				}
			}
			
			return 1;
		}
	}	
	
	return 0;
}

static int etp_server_validate_ftp_data_connection_ip(EVERYTHING_PLUGIN_OS_WINSOCK_SOCKET socket_handle,struct everything_plugin_os_winsock_sockaddr *addr)
{
	if (etp_server_check_ftp_data_connection_ip)
	{
		struct everything_plugin_os_winsock_sockaddr_storage peer_addr;
		int peer_addrlen;

		peer_addrlen = sizeof(struct everything_plugin_os_winsock_sockaddr_storage);

		if (everything_plugin_os_winsock_getpeername(socket_handle,(struct everything_plugin_os_winsock_sockaddr *)&peer_addr,&peer_addrlen) != EVERYTHING_PLUGIN_OS_WINSOCK_SOCKET_ERROR)
		{
			return etp_server_addr_is_host_equal((struct everything_plugin_os_winsock_sockaddr *)&peer_addr,addr);
		}
	
		return 0;
	}
	
	return 1;
}

// port
static void etp_server_client_PORT(etp_server_client_t *c,const everything_plugin_utf8_t *address)
{
	int b[6];
	const everything_plugin_utf8_t *p;
	int argi;

	p = address;
		
	for(argi=0;argi<6;argi++)
	{
		const everything_plugin_utf8_t *start;
		
		start = p;
		
		while(*p)
		{
			if (*p == ',') 
			{
				break;
			}
			
			p++;
		}
		
		b[argi] = everything_plugin_utf8_string_to_int(start);

		if (*p)
		{
			// skip the ','
			p++;
		}
	}

	// close existing data connection.
	etp_server_close_data(c);

	// port MUST match client ip
	if (etp_server_check_ftp_data_connection_ip)
	{
		struct everything_plugin_os_sockaddr_in data_addr;
		
		data_addr.sin_addr.S_un.S_un_b.s_b1 = b[0];
		data_addr.sin_addr.S_un.S_un_b.s_b2 = b[1];
		data_addr.sin_addr.S_un.S_un_b.s_b3 = b[2];
		data_addr.sin_addr.S_un.S_un_b.s_b4 = b[3];
		data_addr.sin_family = EVERYTHING_PLUGIN_OS_WINSOCK_AF_INET;

		if (!etp_server_addr_is_host_equal((struct everything_plugin_os_winsock_sockaddr *)&c->control_addr,(struct everything_plugin_os_winsock_sockaddr *)&data_addr))
		{
			etp_server_client_printf(c,(const everything_plugin_utf8_t *)"500 Failed to connect to IP.\r\n");
			
			return;
		}
	}

	c->data_connection_type = ETP_SERVER_CLIENT_DATA_CONNECTION_TYPE_PORT;
	c->data_is_connected = 0;
	
	c->data_connection_data.port.ip[0] = b[0];
	c->data_connection_data.port.ip[1] = b[1];
	c->data_connection_data.port.ip[2] = b[2];
	c->data_connection_data.port.ip[3] = b[3];
	c->data_connection_data.port.port = (b[4]<<8) | b[5];
		
	etp_server_client_printf(c,(const everything_plugin_utf8_t *)"200 Port command successful.\r\n");
}

// pasv mode
static void etp_server_PASV(etp_server_client_t *c)
{
	// close existing data connection.
	etp_server_close_data(c);
	
    // create the pasv socket.
    if (!c->is_ipv6)
    {
		c->data_connection_data.pasv.socket_handle = everything_plugin_os_winsock_socket(EVERYTHING_PLUGIN_OS_WINSOCK_PF_INET,EVERYTHING_PLUGIN_OS_WINSOCK_SOCK_STREAM,EVERYTHING_PLUGIN_OS_WINSOCK_IPPROTO_TCP);
		if (c->data_connection_data.pasv.socket_handle != EVERYTHING_PLUGIN_OS_WINSOCK_INVALID_SOCKET) 
		{
			int bindret;

			everything_plugin_network_set_tcp_nodelay(c->data_connection_data.pasv.socket_handle);
			everything_plugin_network_set_keepalive(c->data_connection_data.pasv.socket_handle);

			everything_plugin_os_winsock_WSAAsyncSelect(c->data_connection_data.pasv.socket_handle,_etp_server->hwnd,ETP_SERVER_WM_PASV,EVERYTHING_PLUGIN_OS_WINSOCK_FD_ACCEPT|EVERYTHING_PLUGIN_OS_WINSOCK_FD_CLOSE);
			
			{		
				struct everything_plugin_os_sockaddr_in data_sockaddr;
				
				everything_plugin_os_zero_memory(&data_sockaddr,sizeof(struct everything_plugin_os_sockaddr_in));
				
				data_sockaddr.sin_family = EVERYTHING_PLUGIN_OS_WINSOCK_AF_INET;
				data_sockaddr.sin_addr.S_un.S_addr = EVERYTHING_PLUGIN_OS_WINSOCK_INADDR_ANY;
				data_sockaddr.sin_port = 0;
				
				bindret = everything_plugin_os_winsock_bind(c->data_connection_data.pasv.socket_handle,(struct everything_plugin_os_winsock_sockaddr *)&data_sockaddr,sizeof(struct everything_plugin_os_sockaddr_in));
			}

			// Associate the local address with WinSocket.
			if (bindret != EVERYTHING_PLUGIN_OS_WINSOCK_SOCKET_ERROR) 
			{
				// listen for incoming connections	
				if (everything_plugin_os_winsock_listen(c->data_connection_data.pasv.socket_handle,EVERYTHING_PLUGIN_OS_WINSOCK_SOMAXCONN) != EVERYTHING_PLUGIN_OS_WINSOCK_SOCKET_ERROR)
				{
					struct everything_plugin_os_sockaddr_in control_addr;
					int control_addrlen;
				
					control_addrlen = sizeof(struct everything_plugin_os_sockaddr_in);
				
					// use the connected socket handle for the IP
					// but use the correct port
					if (everything_plugin_os_winsock_getsockname(c->control_socket,(struct everything_plugin_os_winsock_sockaddr *)&control_addr,&control_addrlen) != EVERYTHING_PLUGIN_OS_WINSOCK_SOCKET_ERROR)
					{
						if (control_addr.sin_family == EVERYTHING_PLUGIN_OS_WINSOCK_AF_INET)
						{
							struct everything_plugin_os_sockaddr_in pasv_addr;
							int pasv_addrlen;
						
							pasv_addrlen = sizeof(struct everything_plugin_os_sockaddr_in);
						
							// the pasv socket addr will still be unknown because nothing is connected to it.
							if (everything_plugin_os_winsock_getsockname(c->data_connection_data.pasv.socket_handle,(struct everything_plugin_os_winsock_sockaddr *)&pasv_addr,&pasv_addrlen) != EVERYTHING_PLUGIN_OS_WINSOCK_SOCKET_ERROR)
							{
								if (pasv_addr.sin_family == EVERYTHING_PLUGIN_OS_WINSOCK_AF_INET)
								{
									c->data_connection_type = ETP_SERVER_CLIENT_DATA_CONNECTION_TYPE_PASV;
									
									etp_server_client_printf(c,(const everything_plugin_utf8_t *)"227 Entering Passive Mode. (%d,%d,%d,%d,%d,%d)\r\n",
										control_addr.sin_addr.S_un.S_un_b.s_b1,control_addr.sin_addr.S_un.S_un_b.s_b2,control_addr.sin_addr.S_un.S_un_b.s_b3,control_addr.sin_addr.S_un.S_un_b.s_b4,
										pasv_addr.sin_port & 0xff,(pasv_addr.sin_port >> 8) & 0xff);
								}
								else
								{
									etp_server_client_printf(c,(const everything_plugin_utf8_t *)"500 Failed to enter passive mode, invalid pasv family %d\r\n",pasv_addr.sin_family);

									etp_server_close_data(c);
								}
							}
							else
							{
								etp_server_client_printf(c,(const everything_plugin_utf8_t *)"500 Failed to enter passive mode, pasv getsockname error %d\r\n",everything_plugin_os_winsock_WSAGetLastError());

								etp_server_close_data(c);
							}
						}
						else
						{
							etp_server_client_printf(c,(const everything_plugin_utf8_t *)"500 Failed to enter passive mode, invalid control family %d\r\n",control_addr.sin_family);

							etp_server_close_data(c);
						}
					}
					else
					{
						etp_server_client_printf(c,(const everything_plugin_utf8_t *)"500 Failed to enter passive mode, control getsockname error %d\r\n",everything_plugin_os_winsock_WSAGetLastError());

						etp_server_close_data(c);
					}
				}
				else
				{
					etp_server_client_printf(c,(const everything_plugin_utf8_t *)"500 Failed to enter passive mode, listen error %d\r\n",everything_plugin_os_winsock_WSAGetLastError());

					etp_server_close_data(c);
				}
			}
			else
			{
				etp_server_client_printf(c,(const everything_plugin_utf8_t *)"500 Failed to enter passive mode, bind error %d\r\n",everything_plugin_os_winsock_WSAGetLastError());

				etp_server_close_data(c);
			}
		}
		else
		{
			etp_server_client_printf(c,(const everything_plugin_utf8_t *)"500 Failed to enter passive mode, socket error %d.\r\n",everything_plugin_os_winsock_WSAGetLastError());

			etp_server_close_data(c);
		}
	}
	else
	{
		etp_server_client_printf(c,(const everything_plugin_utf8_t *)"500 PASV is not available when using IPv6, please use IPv4 or EPSV.\r\n",everything_plugin_os_winsock_WSAGetLastError());

		etp_server_close_data(c);
	}
}

static void etp_server_client_LIST(etp_server_client_t *c,const everything_plugin_utf8_t *ftp_pathname)
{
	everything_plugin_utf8_buf_t filename_cbuf;
	
	everything_plugin_utf8_buf_init(&filename_cbuf);
	
	// parse ftp_pathname
	{
		everything_plugin_utf8_buf_t ftp_pathname_cbuf;
		everything_plugin_utf8_t *d;
		const everything_plugin_utf8_t *p;
		int is_quote;
		int wasws;

		// ignore options
		// > LIST -la
		// from total commander
		// ignore ls parameters.
		everything_plugin_utf8_buf_init(&ftp_pathname_cbuf);

		everything_plugin_utf8_buf_copy_utf8_string(&ftp_pathname_cbuf,ftp_pathname);

		// ignore -? commands.
		d = ftp_pathname_cbuf.buf;
		p = ftp_pathname_cbuf.buf;
		is_quote = 0;
		wasws = 1;
		
		while(*p)
		{
			if (*p == '"')
			{
				is_quote = !is_quote;
				p++;
			}
			else
			if ((!is_quote) && (everything_plugin_unicode_is_ascii_ws(*p)))
			{
				wasws = 1;
				p++;
			}
			else
			if ((wasws) && (*p == '-'))
			{
				// skip option.
				while(*p)
				{
					if (everything_plugin_unicode_is_ascii_ws(*p)) break;
					
					p++;
				}
			}
			else
			{
				// add to buf.
				*d++ = *p++;
			}
		}
		
		*d = 0;
		
		etp_server_get_full_filename(c,&filename_cbuf,ftp_pathname_cbuf.buf);

		everything_plugin_utf8_buf_kill(&ftp_pathname_cbuf);
	}
	
	if ((!*filename_cbuf.buf) || (everything_plugin_db_folder_exists(_etp_server->db,filename_cbuf.buf)))
	{
		// is the data connection initiated?
		c->data_type = ETP_SERVER_CLIENT_DATA_TYPE_LIST;
		c->data.list.chunk_start = 0;
		c->data.list.chunk_last = 0;
		c->data.list.remaining = 0;
		c->data.list.path = everything_plugin_utf8_string_alloc_utf8_string(filename_cbuf.buf);
		c->data_complete = 0;
		
		etp_server_open_data(c);
	}
	else
	{
		etp_server_client_printf(c,(const everything_plugin_utf8_t *)"550 Directory not found.\r\n");
	}

	everything_plugin_utf8_buf_kill(&filename_cbuf);
}

static void etp_server_client_MLSD(etp_server_client_t *c,const everything_plugin_utf8_t *ftp_pathname)
{
	everything_plugin_utf8_buf_t filename_cbuf;
	
	everything_plugin_utf8_buf_init(&filename_cbuf);
	
	// parse ftp_pathname
	etp_server_get_full_filename(c,&filename_cbuf,ftp_pathname);
	
	if ((!*filename_cbuf.buf) || (everything_plugin_db_folder_exists(_etp_server->db,filename_cbuf.buf)))
	{
		// is the data connection initiated?
		c->data_type = ETP_SERVER_CLIENT_DATA_TYPE_MLSD;
		c->data.list.chunk_start = 0;
		c->data.list.chunk_last = 0;
		c->data.list.remaining = 0;
		c->data.list.path = everything_plugin_utf8_string_alloc_utf8_string(filename_cbuf.buf);
		c->data_complete = 0;
		
		etp_server_open_data(c);
	}
	else
	{
		etp_server_client_printf(c,(const everything_plugin_utf8_t *)"550 Directory not found.\r\n");
	}

	everything_plugin_utf8_buf_kill(&filename_cbuf);
}

static void etp_server_client_print_MLST(etp_server_client_t *c,int is_folder,EVERYTHING_PLUGIN_QWORD modify,EVERYTHING_PLUGIN_QWORD size,const everything_plugin_utf8_t *full_ftp_filename)
{
	// root
	etp_server_client_printf(c,(const everything_plugin_utf8_t *)"250-Listing %s\r\n",full_ftp_filename);

	etp_server_client_printf(c,(const everything_plugin_utf8_t *)" type=%s;",is_folder ? (const everything_plugin_utf8_t *)"dir" : (const everything_plugin_utf8_t *)"file");
	
	if (modify != EVERYTHING_PLUGIN_QWORD_MAX)
	{
		SYSTEMTIME st;
		
		// FTP uses UTC.
		// filetimes are already in UTC.
		FileTimeToSystemTime((FILETIME *)&modify,&st);
		
		etp_server_client_printf(c,(const everything_plugin_utf8_t *)"modify=%04d%02d%02d%02d%02d%02d;",st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond);
	}

	if (size != EVERYTHING_PLUGIN_QWORD_MAX)
	{
		etp_server_client_printf(c,(const everything_plugin_utf8_t *)"size=%I64u;",size);
	}

	etp_server_client_printf(c,(const everything_plugin_utf8_t *)" %s\r\n",full_ftp_filename);

	etp_server_client_printf(c,(const everything_plugin_utf8_t *)"250 End.\r\n");
}

static void etp_server_client_MLST(etp_server_client_t *c,const everything_plugin_utf8_t *ftp_filename)
{
	everything_plugin_utf8_buf_t filename_cbuf;
	everything_plugin_fileinfo_fd_t fd;
	
	everything_plugin_utf8_buf_init(&filename_cbuf);
	
	// parse ftp_pathname
	etp_server_get_full_filename(c,&filename_cbuf,ftp_filename);
	
	if (!*filename_cbuf.buf)
	{
		// root
		etp_server_client_print_MLST(c,1,EVERYTHING_PLUGIN_QWORD_MAX,EVERYTHING_PLUGIN_QWORD_MAX,(const everything_plugin_utf8_t *)"/");
	}
	else
	if (everything_plugin_db_get_indexed_fd(_etp_server->db,filename_cbuf.buf,&fd))
	{
		everything_plugin_utf8_buf_t full_ftp_filename_cbuf;
		
		everything_plugin_utf8_buf_init(&full_ftp_filename_cbuf);

		etp_server_get_ftp_filename(c,&full_ftp_filename_cbuf,filename_cbuf.buf);

		etp_server_client_print_MLST(c,(fd.attributes & FILE_ATTRIBUTE_DIRECTORY) ? 1 : 0,fd.date_modified,fd.size,full_ftp_filename_cbuf.buf);

		everything_plugin_utf8_buf_kill(&full_ftp_filename_cbuf);
	}
	else
	{
		etp_server_client_printf(c,(const everything_plugin_utf8_t *)"550 File or Directory not found.\r\n");
	}

	everything_plugin_utf8_buf_kill(&filename_cbuf);
}

static void etp_server_get_full_filename(etp_server_client_t *c,everything_plugin_utf8_buf_t *cbuf,const everything_plugin_utf8_t *ftp_filename)
{
	if (*ftp_filename == '/')
	{
		everything_plugin_utf8_buf_copy_utf8_string(cbuf,ftp_filename + 1);
	}
	else
	{	
		if (*c->working_directory)
		{
			if (*ftp_filename)
			{
				everything_plugin_utf8_buf_path_cat_filename(cbuf,c->working_directory,ftp_filename);
			}
			else
			{
				everything_plugin_utf8_buf_copy_utf8_string(cbuf,c->working_directory);		
			}
		}
		else
		{
			everything_plugin_utf8_buf_copy_utf8_string(cbuf,ftp_filename);		
		}
	}
	
	// convert forward slash to backslash
	{
		everything_plugin_utf8_t *p;
		
		p = cbuf->buf;
		
		while(*p)
		{
			if (*p == '/')
			{
				*p = '\\';
			}
			
			p++;
		}
	}
	
	// remove trailing backslash.
	{
		everything_plugin_utf8_t *p;
		
		p = cbuf->buf;
		
		while(*p)
		{
			if ((*p == '\\') && (!p[1]))
			{
				*p = 0;
				
				break;
			}
			
			p++;
		}	
	}
	
	everything_plugin_utf8_buf_path_canonicalize(cbuf);
}

// send ascii data to data connection
static void etp_server_client_A(etp_server_client_t *c,const everything_plugin_utf8_t *format,...)
{
	everything_plugin_utf8_buf_t cbuf;
	
	everything_plugin_utf8_buf_init(&cbuf);
	
	{
		va_list argptr;
			
		va_start(argptr,format);
	
		everything_plugin_utf8_buf_vprintf(&cbuf,format,argptr);

		va_end(argptr);
	}
	
	if (c->utf8_on)
	{
		etp_server_list_chunk_add(c,cbuf.buf,cbuf.len);
	}
	else
	{
		everything_plugin_ansi_buf_t acbuf;
		
		everything_plugin_ansi_buf_init(&acbuf);

		everything_plugin_ansi_buf_copy_utf8_string(&acbuf,cbuf.buf);
		
		etp_server_list_chunk_add(c,acbuf.buf,acbuf.len);
		
		everything_plugin_ansi_buf_kill(&acbuf);
	}

	everything_plugin_utf8_buf_kill(&cbuf);
}

// send ascii data to data connection
static void etp_server_client_UTF8(etp_server_client_t *c,const everything_plugin_utf8_t *format,...)
{
	everything_plugin_utf8_buf_t cbuf;
	
	everything_plugin_utf8_buf_init(&cbuf);
	
	{
		va_list argptr;
			
		va_start(argptr,format);
	
		everything_plugin_utf8_buf_vprintf(&cbuf,format,argptr);

		va_end(argptr);
	}
	
	etp_server_list_chunk_add(c,cbuf.buf,cbuf.len);

	everything_plugin_utf8_buf_kill(&cbuf);
}

static void etp_server_list_chunk_add(etp_server_client_t *c,const void *data,uintptr_t size)
{
	const char *d;
	
	d = data;
	
	for(;;)
	{
		if (!size) break;
		
		// grow.
		if ((!c->data.list.chunk_last) || (c->data.list.chunk_last->size == ETP_SERVER_LIST_CHUNK_SIZE - sizeof(etp_server_list_chunk_t)))
		{
			etp_server_list_chunk_t *list_chunk;
			
			// new chunk.
			list_chunk = everything_plugin_mem_alloc(ETP_SERVER_LIST_CHUNK_SIZE);
			list_chunk->size = 0;

			if (c->data.list.chunk_start)			
			{
				c->data.list.chunk_last->next = list_chunk;
			}
			else
			{
				c->data.list.chunk_start = list_chunk;
			}
			
			list_chunk->next = 0;
			c->data.list.chunk_last = list_chunk;
		}
		
		// copy some data to chunk.
		{
			uintptr_t copy_size;
			
			copy_size = size;
			
			if (copy_size > ETP_SERVER_LIST_CHUNK_SIZE - sizeof(etp_server_list_chunk_t) - c->data.list.chunk_last->size)
			{
				copy_size = ETP_SERVER_LIST_CHUNK_SIZE - sizeof(etp_server_list_chunk_t) - c->data.list.chunk_last->size;
			}
			
			everything_plugin_os_copy_memory(((char *)(c->data.list.chunk_last + 1)) + c->data.list.chunk_last->size,d,copy_size);
			
			// copy_size is limited to ETP_SERVER_LIST_CHUNK_SIZE above.
			c->data.list.chunk_last->size += (DWORD)copy_size;
			d += copy_size;
			size -= copy_size;
		}
	}
}

// change working directory.
static void etp_server_client_CWD(etp_server_client_t *c,const everything_plugin_utf8_t *ftp_path)
{
	everything_plugin_utf8_buf_t path_cbuf;
	
	// check for root /
	everything_plugin_utf8_buf_init(&path_cbuf);
	
	etp_server_get_full_filename(c,&path_cbuf,ftp_path);
	
	// is it a valid folder?
	if ((!*path_cbuf.buf) || (everything_plugin_db_folder_exists(_etp_server->db,path_cbuf.buf)))
	{	
		everything_plugin_utf8_buf_t pwd_cbuf;
		
		everything_plugin_utf8_buf_init(&pwd_cbuf);
		
		c->working_directory = everything_plugin_utf8_string_realloc_utf8_string(c->working_directory,path_cbuf.buf);
		
		etp_server_get_ftp_filename(c,&pwd_cbuf,c->working_directory);
		
		// ok
		etp_server_client_printf(c,(const everything_plugin_utf8_t *)"250 CWD successful. \"%s\" is current directory.\r\n",pwd_cbuf.buf);

		everything_plugin_utf8_buf_kill(&pwd_cbuf);
	}
	else
	{
		etp_server_client_printf(c,(const everything_plugin_utf8_t *)"550 Directory not found.\r\n");
	}
	
	everything_plugin_utf8_buf_kill(&path_cbuf);
}

static void etp_server_get_ftp_filename(etp_server_client_t *c,everything_plugin_utf8_buf_t *cbuf,const everything_plugin_utf8_t *filename)
{
	everything_plugin_utf8_buf_printf(cbuf,(const everything_plugin_utf8_t *)"/%s",filename);
	
	{
		everything_plugin_utf8_t *p;
		
		// skip first '/'
		p = cbuf->buf + 1;
		
		// allow \\ prefix.
		while(*p)
		{
			if (*p != '\\')
			{
				break;
			}
			
			p++;
		}
		
		while(*p)
		{
			if (*p == '\\')
			{
				*p = '/';
			}
			
			p++;
		}
	}
}	

static void etp_server_client_everything(etp_server_client_t *c,const everything_plugin_utf8_t *command,const everything_plugin_utf8_t *param)
{
	if (everything_plugin_utf8_string_compare_nocase_s_sla(command,(const everything_plugin_utf8_t *)"case") == 0)
	{
		c->match_case = everything_plugin_utf8_string_to_int(param) ? 1 : 0;
		
		etp_server_client_printf(c,(const everything_plugin_utf8_t *)"200 Case set to (%d).\r\n",c->match_case);
	}
	else
	if (everything_plugin_utf8_string_compare_nocase_s_sla(command,(const everything_plugin_utf8_t *)"whole_word") == 0)
	{
		c->match_whole_word = everything_plugin_utf8_string_to_int(param) ? 1 : 0;
		
		etp_server_client_printf(c,(const everything_plugin_utf8_t *)"200 Whole word set to (%d).\r\n",c->match_whole_word);
	}
	else
	if (everything_plugin_utf8_string_compare_nocase_s_sla(command,(const everything_plugin_utf8_t *)"path") == 0)
	{
		c->match_path = everything_plugin_utf8_string_to_int(param) ? 1 : 0;
		
		etp_server_client_printf(c,(const everything_plugin_utf8_t *)"200 Path set to (%d).\r\n",c->match_path);
	}
	else
	if (everything_plugin_utf8_string_compare_nocase_s_sla(command,(const everything_plugin_utf8_t *)"diacritics") == 0)
	{
		c->match_diacritics = everything_plugin_utf8_string_to_int(param) ? 1 : 0;
		
		etp_server_client_printf(c,(const everything_plugin_utf8_t *)"200 Diacritics set to (%d).\r\n",c->match_diacritics);
	}
	else
	if (everything_plugin_utf8_string_compare_nocase_s_sla(command,(const everything_plugin_utf8_t *)"prefix") == 0)
	{
		c->match_prefix = everything_plugin_utf8_string_to_int(param) ? 1 : 0;
		
		etp_server_client_printf(c,(const everything_plugin_utf8_t *)"200 Prefix set to (%d).\r\n",c->match_prefix);
	}
	else
	if (everything_plugin_utf8_string_compare_nocase_s_sla(command,(const everything_plugin_utf8_t *)"suffix") == 0)
	{
		c->match_suffix = everything_plugin_utf8_string_to_int(param) ? 1 : 0;
		
		etp_server_client_printf(c,(const everything_plugin_utf8_t *)"200 Suffix set to (%d).\r\n",c->match_suffix);
	}
	else
	if (everything_plugin_utf8_string_compare_nocase_s_sla(command,(const everything_plugin_utf8_t *)"ignore_punctuation") == 0)
	{
		c->ignore_punctuation = everything_plugin_utf8_string_to_int(param) ? 1 : 0;
		
		etp_server_client_printf(c,(const everything_plugin_utf8_t *)"200 Ignore punctuation set to (%d).\r\n",c->ignore_punctuation);
	}
	else
	if (everything_plugin_utf8_string_compare_nocase_s_sla(command,(const everything_plugin_utf8_t *)"ignore_whitespace") == 0)
	{
		c->ignore_whitespace = everything_plugin_utf8_string_to_int(param) ? 1 : 0;
		
		etp_server_client_printf(c,(const everything_plugin_utf8_t *)"200 Ignore whitespace set to (%d).\r\n",c->ignore_whitespace);
	}
	else
	if (everything_plugin_utf8_string_compare_nocase_s_sla(command,(const everything_plugin_utf8_t *)"regex") == 0)
	{
		c->match_regex = everything_plugin_utf8_string_to_int(param) ? 1 : 0;
		
		etp_server_client_printf(c,(const everything_plugin_utf8_t *)"200 Regex set to (%d).\r\n",c->match_regex);
	}
	else
	if (everything_plugin_utf8_string_compare_nocase_s_sla(command,(const everything_plugin_utf8_t *)"hide_empty_search_results") == 0)
	{
		c->hide_empty_search_results = everything_plugin_utf8_string_to_int(param) ? 1 : 0;
		
		etp_server_client_printf(c,(const everything_plugin_utf8_t *)"200 Hide empty search results set to (%d).\r\n",c->hide_empty_search_results);
	}
	else
	if (everything_plugin_utf8_string_compare_nocase_s_sla(command,(const everything_plugin_utf8_t *)"search") == 0)
	{
		c->search_string = everything_plugin_utf8_string_realloc_utf8_string(c->search_string,param);
		
		etp_server_client_printf(c,(const everything_plugin_utf8_t *)"200 Search set to (%s).\r\n",c->search_string);
	}
	else
	if (everything_plugin_utf8_string_compare_nocase_s_sla(command,(const everything_plugin_utf8_t *)"filter_search") == 0)
	{
		c->filter_search = everything_plugin_utf8_string_realloc_utf8_string(c->filter_search,param);
		
		etp_server_client_printf(c,(const everything_plugin_utf8_t *)"200 Filter search set to (%s).\r\n",c->filter_search);
	}
	else
	if (everything_plugin_utf8_string_compare_nocase_s_sla(command,(const everything_plugin_utf8_t *)"filter_case") == 0)
	{
		if (everything_plugin_utf8_string_to_int(param))
		{
			c->filter_flags |= EVERYTHING_PLUGIN_FILTER_FLAG_CASE;
		}
		else
		{
			c->filter_flags &= ~EVERYTHING_PLUGIN_FILTER_FLAG_CASE;
		}
		
		etp_server_client_printf(c,(const everything_plugin_utf8_t *)"200 Filter case set to (%d).\r\n",(c->filter_flags & EVERYTHING_PLUGIN_FILTER_FLAG_CASE) ? 1 : 0);
	}
	else
	if (everything_plugin_utf8_string_compare_nocase_s_sla(command,(const everything_plugin_utf8_t *)"filter_diacritics") == 0)
	{
		if (everything_plugin_utf8_string_to_int(param))
		{
			c->filter_flags |= EVERYTHING_PLUGIN_FILTER_FLAG_DIACRITICS;
		}
		else
		{
			c->filter_flags &= ~EVERYTHING_PLUGIN_FILTER_FLAG_DIACRITICS;
		}
		
		etp_server_client_printf(c,(const everything_plugin_utf8_t *)"200 Filter diacritics set to (%d).\r\n",(c->filter_flags & EVERYTHING_PLUGIN_FILTER_FLAG_DIACRITICS) ? 1 : 0);
	}
	else
	if (everything_plugin_utf8_string_compare_nocase_s_sla(command,(const everything_plugin_utf8_t *)"filter_prefix") == 0)
	{
		if (everything_plugin_utf8_string_to_int(param))
		{
			c->filter_flags |= EVERYTHING_PLUGIN_FILTER_FLAG_PREFIX;
		}
		else
		{
			c->filter_flags &= ~EVERYTHING_PLUGIN_FILTER_FLAG_PREFIX;
		}
		
		etp_server_client_printf(c,(const everything_plugin_utf8_t *)"200 Filter prefix set to (%d).\r\n",(c->filter_flags & EVERYTHING_PLUGIN_FILTER_FLAG_PREFIX) ? 1 : 0);
	}
	else
	if (everything_plugin_utf8_string_compare_nocase_s_sla(command,(const everything_plugin_utf8_t *)"filter_suffix") == 0)
	{
		if (everything_plugin_utf8_string_to_int(param))
		{
			c->filter_flags |= EVERYTHING_PLUGIN_FILTER_FLAG_SUFFIX;
		}
		else
		{
			c->filter_flags &= ~EVERYTHING_PLUGIN_FILTER_FLAG_SUFFIX;
		}
		
		etp_server_client_printf(c,(const everything_plugin_utf8_t *)"200 Filter ignore punctuation set to (%d).\r\n",(c->filter_flags & EVERYTHING_PLUGIN_FILTER_FLAG_IGNORE_PUNCTUATION) ? 1 : 0);
	}
	else
	if (everything_plugin_utf8_string_compare_nocase_s_sla(command,(const everything_plugin_utf8_t *)"filter_ignore_punctuation") == 0)
	{
		if (everything_plugin_utf8_string_to_int(param))
		{
			c->filter_flags |= EVERYTHING_PLUGIN_FILTER_FLAG_IGNORE_PUNCTUATION;
		}
		else
		{
			c->filter_flags &= ~EVERYTHING_PLUGIN_FILTER_FLAG_IGNORE_PUNCTUATION;
		}
		
		etp_server_client_printf(c,(const everything_plugin_utf8_t *)"200 Filter ignore punctuation set to (%d).\r\n",(c->filter_flags & EVERYTHING_PLUGIN_FILTER_FLAG_IGNORE_PUNCTUATION) ? 1 : 0);
	}
	else
	if (everything_plugin_utf8_string_compare_nocase_s_sla(command,(const everything_plugin_utf8_t *)"filter_ignore_whitespace") == 0)
	{
		if (everything_plugin_utf8_string_to_int(param))
		{
			c->filter_flags |= EVERYTHING_PLUGIN_FILTER_FLAG_IGNORE_WHITESPACE;
		}
		else
		{
			c->filter_flags &= ~EVERYTHING_PLUGIN_FILTER_FLAG_IGNORE_WHITESPACE;
		}
		
		etp_server_client_printf(c,(const everything_plugin_utf8_t *)"200 Filter ignore whitespace set to (%d).\r\n",(c->filter_flags & EVERYTHING_PLUGIN_FILTER_FLAG_IGNORE_WHITESPACE) ? 1 : 0);
	}
	else
	if (everything_plugin_utf8_string_compare_nocase_s_sla(command,(const everything_plugin_utf8_t *)"filter_path") == 0)
	{
		if (everything_plugin_utf8_string_to_int(param))
		{
			c->filter_flags |= EVERYTHING_PLUGIN_FILTER_FLAG_PATH;
		}
		else
		{
			c->filter_flags &= ~EVERYTHING_PLUGIN_FILTER_FLAG_PATH;
		}
		
		etp_server_client_printf(c,(const everything_plugin_utf8_t *)"200 Filter path set to (%d).\r\n",(c->filter_flags & EVERYTHING_PLUGIN_FILTER_FLAG_PATH) ? 1 : 0);
	}
	else
	if (everything_plugin_utf8_string_compare_nocase_s_sla(command,(const everything_plugin_utf8_t *)"filter_regex") == 0)
	{
		if (everything_plugin_utf8_string_to_int(param))
		{
			c->filter_flags |= EVERYTHING_PLUGIN_FILTER_FLAG_REGEX;
		}
		else
		{
			c->filter_flags &= ~EVERYTHING_PLUGIN_FILTER_FLAG_REGEX;
		}
		
		etp_server_client_printf(c,(const everything_plugin_utf8_t *)"200 Filter regex set to (%d).\r\n",(c->filter_flags & EVERYTHING_PLUGIN_FILTER_FLAG_REGEX) ? 1 : 0);
	}
	else
	if (everything_plugin_utf8_string_compare_nocase_s_sla(command,(const everything_plugin_utf8_t *)"filter_whole_word") == 0)
	{
		if (everything_plugin_utf8_string_to_int(param))
		{
			c->filter_flags |= EVERYTHING_PLUGIN_FILTER_FLAG_WHOLEWORD;
		}
		else
		{
			c->filter_flags &= ~EVERYTHING_PLUGIN_FILTER_FLAG_WHOLEWORD;
		}
		
		etp_server_client_printf(c,(const everything_plugin_utf8_t *)"200 Filter whole word set to (%d).\r\n",(c->filter_flags & EVERYTHING_PLUGIN_FILTER_FLAG_WHOLEWORD) ? 1 : 0);
	}
	else
	if (everything_plugin_utf8_string_compare_nocase_s_sla(command,(const everything_plugin_utf8_t *)"sort") == 0)
	{
		const everything_plugin_property_t *sort_property_type;
		int sort_ascending;
		
		sort_property_type = etp_server_get_sort_property_from_name(param);
		sort_ascending = etp_server_get_sort_ascending_from_name(param);
		
		if (!sort_property_type)
		{
			etp_server_client_printf(c,(const everything_plugin_utf8_t *)"500 Unknown sort type.\r\n");
		}
		else
		{
			c->sort_property_type = sort_property_type;
			c->sort_ascending = sort_ascending;
			etp_server_client_printf(c,(const everything_plugin_utf8_t *)"200 Sort set to (%s).\r\n",param);
		}
	}
	else
	if (everything_plugin_utf8_string_compare_nocase_s_sla(command,(const everything_plugin_utf8_t *)"offset") == 0)
	{
		c->offset = everything_plugin_utf8_string_to_dword(param);
		
		etp_server_client_printf(c,(const everything_plugin_utf8_t *)"200 Offset set to (%u).\r\n",c->offset);
	}
	else
	if (everything_plugin_utf8_string_compare_nocase_s_sla(command,(const everything_plugin_utf8_t *)"count") == 0)
	{
		c->count = everything_plugin_utf8_string_to_dword(param);
		
		etp_server_client_printf(c,(const everything_plugin_utf8_t *)"200 Count set to (%u).\r\n",c->count);
	}
	else
	if (everything_plugin_utf8_string_compare_nocase_s_sla(command,(const everything_plugin_utf8_t *)"size_column") == 0)
	{
		c->size_column = everything_plugin_utf8_string_to_int(param);
		
		etp_server_client_printf(c,(const everything_plugin_utf8_t *)"200 Size column set to (%u).\r\n",c->size_column);
	}
	else
	if (everything_plugin_utf8_string_compare_nocase_s_sla(command,(const everything_plugin_utf8_t *)"attributes_column") == 0)
	{
		c->attributes_column = everything_plugin_utf8_string_to_int(param);
		
		etp_server_client_printf(c,(const everything_plugin_utf8_t *)"200 Attributes column set to (%u).\r\n",c->attributes_column);
	}
	else
	if (everything_plugin_utf8_string_compare_nocase_s_sla(command,(const everything_plugin_utf8_t *)"date_modified_column") == 0)
	{
		c->date_modified_column = everything_plugin_utf8_string_to_int(param);
		
		etp_server_client_printf(c,(const everything_plugin_utf8_t *)"200 Date modified column set to (%u).\r\n",c->date_modified_column);
	}
	else
	if (everything_plugin_utf8_string_compare_nocase_s_sla(command,(const everything_plugin_utf8_t *)"date_created_column") == 0)
	{
		c->date_created_column = everything_plugin_utf8_string_to_int(param);
		
		etp_server_client_printf(c,(const everything_plugin_utf8_t *)"200 Date created column set to (%u).\r\n",c->date_created_column);
	}
	else
	if (everything_plugin_utf8_string_compare_nocase_s_sla(command,(const everything_plugin_utf8_t *)"path_column") == 0)
	{
		c->path_column = everything_plugin_utf8_string_to_int(param);
		
		etp_server_client_printf(c,(const everything_plugin_utf8_t *)"200 Path column set to (%u).\r\n",c->path_column);
	}
	else
	if (everything_plugin_utf8_string_compare_nocase_s_sla(command,(const everything_plugin_utf8_t *)"file_list_filename_column") == 0)
	{
		c->file_list_filename_column = everything_plugin_utf8_string_to_int(param);
		
		etp_server_client_printf(c,(const everything_plugin_utf8_t *)"200 File list filename column set to (%u).\r\n",c->file_list_filename_column);
	}
	else
	if (everything_plugin_utf8_string_compare_nocase_s_sla(command,(const everything_plugin_utf8_t *)"date_recently_changed_column") == 0)
	{
		c->date_recently_changed_column = everything_plugin_utf8_string_to_int(param);
		
		etp_server_client_printf(c,(const everything_plugin_utf8_t *)"200 Date recently changed column set to (%u).\r\n",c->date_recently_changed_column);
	}
	else
	if (everything_plugin_utf8_string_compare_nocase_s_sla(command,(const everything_plugin_utf8_t *)"query") == 0)
	{
		// is it the same as the current query?
		// this cache can be really old if we have rebuilt the database.
		if ((c->is_current_query) && 
			(c->current_match_case == c->match_case) && 
			(c->current_match_whole_word == c->match_whole_word) && 
			(c->current_match_path == c->match_path) && 
			(c->current_match_diacritics == c->match_diacritics) &&
			(c->current_match_prefix == c->match_prefix) &&
			(c->current_match_suffix == c->match_suffix) &&
			(c->current_ignore_punctuation == c->ignore_punctuation) &&
			(c->current_ignore_whitespace == c->ignore_whitespace) &&
			(c->current_match_regex == c->match_regex) &&
			(c->current_size_column == c->size_column) &&
			(c->current_file_list_filename_column == c->file_list_filename_column) &&
			(c->current_date_recently_changed_column == c->date_recently_changed_column) &&
			(c->current_attributes_column == c->attributes_column) &&
			(c->current_date_modified_column == c->date_modified_column) &&
			(c->current_date_created_column == c->date_created_column) &&
			(c->current_path_column == c->path_column) &&
			(c->current_hide_empty_search_results == c->hide_empty_search_results) &&
			(everything_plugin_utf8_string_compare(c->current_search_string,c->search_string) == 0) &&
			(everything_plugin_utf8_string_compare(c->current_filter_search,c->filter_search) == 0) &&
			(c->current_filter_search_flags == c->filter_flags)
		)
		{
			if ((c->current_sort_column_type == c->sort_property_type) && (!!c->current_sort_ascending == !!c->sort_ascending))
			{
				// same query and sort.
				etp_server_send_query_results(c);
			}
			else
			{
				c->is_query = 1;
				c->current_sort_column_type = c->sort_property_type;
				c->current_sort_ascending = c->sort_ascending;
			
				// sort it first.
				everything_plugin_db_query_sort(c->db_query,c->sort_property_type,c->sort_ascending,NULL,0,NULL,0,0,0,EVERYTHING_PLUGIN_DB_QUERY_FIND_DUPLICATES_NONE,0);
			}
		}
		else
		{
			c->is_query = 1;

			c->current_match_case = c->match_case;
			c->current_match_whole_word = c->match_whole_word;
			c->current_match_path = c->match_path;
			c->current_match_diacritics = c->match_diacritics;
			c->current_match_prefix = c->match_prefix;
			c->current_match_suffix = c->match_suffix;
			c->current_ignore_punctuation = c->ignore_punctuation;
			c->current_ignore_whitespace = c->ignore_whitespace;
			c->current_match_regex = c->match_regex;
			c->current_size_column = c->size_column;
			c->current_file_list_filename_column = c->file_list_filename_column;
			c->current_date_recently_changed_column = c->date_recently_changed_column;
			c->current_attributes_column = c->attributes_column;
			c->current_date_modified_column = c->date_modified_column;
			c->current_date_created_column = c->date_created_column;
			c->current_path_column = c->path_column;
			c->current_hide_empty_search_results = c->hide_empty_search_results;
			c->current_search_string = everything_plugin_utf8_string_realloc_utf8_string(c->current_search_string,c->search_string);
			c->current_filter_search = everything_plugin_utf8_string_realloc_utf8_string(c->current_filter_search,c->filter_search);
			c->current_filter_search_flags = c->filter_flags;
			c->current_sort_column_type = c->sort_property_type;
			c->current_sort_ascending = c->sort_ascending;
			c->is_current_query = 1;
		
			// start the query.
			everything_plugin_db_query_search2(
				c->db_query,
				c->match_case,
				c->match_whole_word,
				c->match_path,
				c->match_diacritics,
				c->match_prefix,
				c->match_suffix,
				c->ignore_punctuation,
				c->ignore_whitespace,
				c->match_regex,
				c->hide_empty_search_results,
				1,
				1,
				c->search_string,
				c->filter_flags,
				c->filter_search,
				(const everything_plugin_utf8_t *)"",
				NULL,
				0,
				-1,
				1,
				c->sort_property_type,
				c->sort_ascending,
				NULL,
				0,
				NULL,
				0,
				0,
				0,
				0,
				0,
				0,
				0,
				0,
				0,
				etp_server_allow_disk_access,
				0,
				EVERYTHING_PLUGIN_CONFIG_SIZE_STANDARD_JEDEC,
				0,
				1,
				0);
		}
	}
	else
	{
		etp_server_client_printf(c,(const everything_plugin_utf8_t *)"500 Unknown Everything command.\r\n");
	}
}

// change working directory.
static void etp_server_client_CDUP(etp_server_client_t *c)
{
	everything_plugin_utf8_buf_t filename;
	
	// check for root /
	everything_plugin_utf8_buf_init(&filename);
	
	etp_server_get_full_filename(c,&filename,(const everything_plugin_utf8_t *)"..");
	
	{
		everything_plugin_utf8_buf_t pwd_cbuf;
		
		everything_plugin_utf8_buf_init(&pwd_cbuf);
		
		c->working_directory = everything_plugin_utf8_string_realloc_utf8_string(c->working_directory,filename.buf);
		
		etp_server_get_ftp_filename(c,&pwd_cbuf,c->working_directory);
		
		// ok
		etp_server_client_printf(c,(const everything_plugin_utf8_t *)"250 CDUP successful. \"%s\" is current directory.\r\n",pwd_cbuf.buf);

		everything_plugin_utf8_buf_kill(&pwd_cbuf);
	}


	everything_plugin_utf8_buf_kill(&filename);
}

static int etp_server_is_last_line_of_welcome_message(const everything_plugin_utf8_t *p)
{
	while(*p)
	{
		if (*p == '%')
		{
			p++;
			
			if (*p == 'n') return 0;
			
			p++;
		}
		else
		{
			p++;
		}
	}
	
	return 1;
}

// buf can be null
static everything_plugin_utf8_t *_etp_server_utf8_string_append_utf8_string(everything_plugin_utf8_t *buf,everything_plugin_utf8_t *d,const everything_plugin_utf8_t *s)
{
	if (buf)
	{
		return everything_plugin_utf8_string_copy_utf8_string(d,s);
	}
	else
	{
		return (void *)everything_plugin_safe_uintptr_add((uintptr_t)d,everything_plugin_utf8_string_get_length_in_bytes(s));
	}
}

static everything_plugin_utf8_t *etp_server_get_welcome_message(everything_plugin_utf8_t *buf,const everything_plugin_utf8_t *message)
{
	const everything_plugin_utf8_t *p;
	everything_plugin_utf8_t *d;
	
	if (!*message)
	{
		// default message.
		message = (const everything_plugin_utf8_t *)"Welcome to Everything ETP/FTP";
	}

	p = message;
	d = buf;

	for(;;)
	{
		int is_last_line;
		
		is_last_line = etp_server_is_last_line_of_welcome_message(p);
		
		if (is_last_line)
		{
			d = _etp_server_utf8_string_append_utf8_string(buf,d,(const everything_plugin_utf8_t *)"220 ");
		}
		else
		{
			d = _etp_server_utf8_string_append_utf8_string(buf,d,(const everything_plugin_utf8_t *)"220-");
		}
		
		while(*p)
		{
			if (*p == '\r')
			{
				// ignore newlines, MUST use %n
				p++;
			}
			else
			if (*p == '\n')
			{
				// ignore newlines, MUST use %n
				p++;
			}
			else
			if (*p == '%')
			{
				p++;

				if (*p == '%')
				{
					d = _etp_server_utf8_string_append_utf8_string(buf,d,"%");
				}
				else
				if (*p == 'v')
				{
					everything_plugin_utf8_buf_t version_buf;
					
					everything_plugin_utf8_buf_init(&version_buf);
					
					everything_plugin_version_get_text(&version_buf);
				
					d = _etp_server_utf8_string_append_utf8_string(buf,d,version_buf.buf);

					everything_plugin_utf8_buf_kill(&version_buf);
				}
				else
				if (*p == 'n')
				{
					p++;
					
					break;
				}

				p++;
			}
			else
			{
				if (buf)
				{
					*d++ = *p;
				}
				else
				{
					d = (void *)everything_plugin_safe_uintptr_add((uintptr_t)d,1);
				}

				p++;
			}
		}

		// EOL.
		d = _etp_server_utf8_string_append_utf8_string(buf,d,(const everything_plugin_utf8_t *)"\r\n");
		
		if (is_last_line)
		{
			break;
		}
	}
		
	if (buf)
	{
		*d = 0;
	}
	
	return d;
}

// send a file
static void etp_server_SIZE(etp_server_client_t *c,const everything_plugin_utf8_t *ftp_filename)
{
	everything_plugin_utf8_buf_t filename_cbuf;
	everything_plugin_fileinfo_fd_t fd;
	
	// check for root /
	everything_plugin_utf8_buf_init(&filename_cbuf);
	
	etp_server_get_full_filename(c,&filename_cbuf,ftp_filename);

	// check for access
	if (everything_plugin_db_get_indexed_fd(_etp_server->db,filename_cbuf.buf,&fd))
	{
		if (fd.size != EVERYTHING_PLUGIN_QWORD_MAX)
		{
			// success
			etp_server_client_printf(c,(const everything_plugin_utf8_t *)"200 %I64u\r\n",fd.size);
		}
		else
		{
			// invalid size.
			etp_server_client_printf(c,(const everything_plugin_utf8_t *)"550 File not found.\r\n");
		}
	}
	else
	{
		// file not found.
		etp_server_client_printf(c,(const everything_plugin_utf8_t *)"550 File not found.\r\n");
	}	
	
	everything_plugin_utf8_buf_kill(&filename_cbuf);
}

// send File Modification Time
static void etp_server_client_MDTM(etp_server_client_t *c,const everything_plugin_utf8_t *filename)
{
	everything_plugin_utf8_buf_t filename_cbuf;
	everything_plugin_fileinfo_fd_t fd;
	
	// check for root /
	everything_plugin_utf8_buf_init(&filename_cbuf);
	
	etp_server_get_full_filename(c,&filename_cbuf,filename);

	// check for access
	if (everything_plugin_db_get_indexed_fd(_etp_server->db,filename_cbuf.buf,&fd))
	{
		if (fd.date_modified != EVERYTHING_PLUGIN_QWORD_MAX)
		{
			SYSTEMTIME st;

			// FTP uses UTC.
			// filetimes are already in UTC.
			FileTimeToSystemTime((FILETIME *)&fd.date_modified,&st);

			etp_server_client_printf(c,
				(const everything_plugin_utf8_t *)"213 %04d%02d%02d"
				"%02d%02d%02d.%03d\r\n",
				st.wYear,st.wMonth,st.wDay,
				st.wHour,st.wMinute,st.wSecond,st.wMilliseconds);
		}
		else
		{
			// invalid date.
			etp_server_client_printf(c,(const everything_plugin_utf8_t *)"550 File not found.\r\n");
		}
	}
	else
	{
		// file not found.
		etp_server_client_printf(c,(const everything_plugin_utf8_t *)"550 File not found.\r\n");
	}		

	everything_plugin_utf8_buf_kill(&filename_cbuf);
}

// get the file size
// this should really be moved to a thread.
static void etp_server_RETR(etp_server_client_t *c,const everything_plugin_utf8_t *ftp_filename)
{
	EVERYTHING_PLUGIN_QWORD rest;
	
	rest = c->rest;
	
	// reset rest.
	c->rest = 0;
	
	if (etp_server_allow_file_download)
	{
		everything_plugin_utf8_buf_t filename_cbuf;
		
		// check for root /
		everything_plugin_utf8_buf_init(&filename_cbuf);
		
		etp_server_get_full_filename(c,&filename_cbuf,ftp_filename);
	
		// check if db allows this drive
		if (everything_plugin_db_file_exists(_etp_server->db,filename_cbuf.buf))
		{
			HANDLE f;
			
			// try to open it
			f = everything_plugin_os_open_file(filename_cbuf.buf);
			if (f != INVALID_HANDLE_VALUE)
			{
				int goterror;

//everything_plugin_debug_printf((const everything_plugin_utf8_t *)"rest %I64u\n",rest.QuadPart);

				goterror = 0;
				
				if (rest)
				{
					if (!everything_plugin_os_set_file_pointer(f,rest,FILE_BEGIN))
					{
						etp_server_client_printf(c,(const everything_plugin_utf8_t *)"451 Transfer aborted: rest error.\r\n");
						
						goterror = 1;
					}
				}
				

				// restart
				if (!goterror)
				{
					c->data_type = ETP_SERVER_CLIENT_DATA_TYPE_RETR;

					// init data state
					c->data.retr.buffer = everything_plugin_mem_alloc(ETP_SERVER_RETR_CHUNK_SIZE);
					c->data.retr.file = f;
					f = INVALID_HANDLE_VALUE;
					InitializeCriticalSection(&c->data.retr.cs);
					c->data.retr.hevent = everything_plugin_os_event_create();
					c->data.retr.thread = everything_plugin_os_thread_create(etp_server_retr_thread_proc,c);
					c->data.retr.size = 0;
					c->data.retr.remaining = 0;
					c->data.retr.state = 0;
					c->data.retr.abort = 0;
					
					// set state..
					c->data_complete = 0;
					c->data_type = ETP_SERVER_CLIENT_DATA_TYPE_RETR;

					etp_server_open_data(c);
				}
				
				if (f != INVALID_HANDLE_VALUE)	
				{
					CloseHandle(f);
				}
			}
			else
			{
				// CreateFile failed
				etp_server_client_printf(c,(const everything_plugin_utf8_t *)"550 File not found.\r\n");

				etp_server_close_data(c);
			}
		}
		else
		{
			// file not found
			etp_server_client_printf(c,(const everything_plugin_utf8_t *)"550 File not found.\r\n");

			etp_server_close_data(c);
		}
		
		everything_plugin_utf8_buf_kill(&filename_cbuf);
	}
	else
	{
		// file download disabled
		etp_server_client_printf(c,(const everything_plugin_utf8_t *)"550 File not found.\r\n");

		etp_server_close_data(c);
	}
}

// close data connection.
static void etp_server_close_data(etp_server_client_t *c)
{
	if (c->data_socket != EVERYTHING_PLUGIN_OS_WINSOCK_INVALID_SOCKET)
	{
		everything_plugin_os_winsock_closesocket(c->data_socket);
		
		c->data_socket = EVERYTHING_PLUGIN_OS_WINSOCK_INVALID_SOCKET;
	}
	
	switch(c->data_type)
	{
		case ETP_SERVER_CLIENT_DATA_TYPE_LIST:
		case ETP_SERVER_CLIENT_DATA_TYPE_MLSD:
		{
			etp_server_list_chunk_t *list_chunk;
			etp_server_list_chunk_t *next_list_chunk;	
			
			list_chunk = c->data.list.chunk_start;
			while(list_chunk)
			{
				next_list_chunk = list_chunk->next;
				
				everything_plugin_mem_free(list_chunk);
				
				list_chunk = next_list_chunk;
			}
			
			everything_plugin_mem_free(c->data.list.path);

			break;
		}
		
		case ETP_SERVER_CLIENT_DATA_TYPE_RETR:
		{
		
			EnterCriticalSection(&c->data.retr.cs);
			c->data.retr.abort = 1;
			LeaveCriticalSection(&c->data.retr.cs);
			
			SetEvent(c->data.retr.hevent);
			
			everything_plugin_os_thread_wait_and_close(c->data.retr.thread);
			CloseHandle(c->data.retr.hevent);

			DeleteCriticalSection(&c->data.retr.cs);		

			CloseHandle(c->data.retr.file);
			everything_plugin_mem_free(c->data.retr.buffer);
			
			break;
		}
	}

	c->data_type = ETP_SERVER_CLIENT_DATA_TYPE_NONE;

	switch(c->data_connection_type)
	{
		case ETP_SERVER_CLIENT_DATA_CONNECTION_TYPE_PASV:
		
			// handle could be closed.
			if (c->data_connection_data.pasv.socket_handle != EVERYTHING_PLUGIN_OS_WINSOCK_INVALID_SOCKET)
			{
				everything_plugin_os_winsock_closesocket(c->data_connection_data.pasv.socket_handle);

				c->data_connection_data.pasv.socket_handle = EVERYTHING_PLUGIN_OS_WINSOCK_INVALID_SOCKET;
			}
			
			break;
	}

	c->data_connection_type = ETP_SERVER_CLIENT_DATA_CONNECTION_TYPE_NONE;
}

// only called when retr buffer is depleted.
// destroyed when client is destroy, so c exists while thread is running
// _etp_server exists while thread is running, since closing the etp_server would close this thread first.
static DWORD WINAPI etp_server_retr_thread_proc(etp_server_client_t *c)
{
	for(;;)
	{
		DWORD num_bytes_read;
		
		// wait for data request.
		WaitForSingleObject(c->data.retr.hevent,INFINITE);

		// reset event		
		ResetEvent(c->data.retr.hevent);
		
		// abort?
		EnterCriticalSection(&c->data.retr.cs);
		
		if (c->data.retr.abort)
		{
			LeaveCriticalSection(&c->data.retr.cs);

			break;
		}
		
		// still data remaining?		
		if (c->data.retr.remaining)
		{
			LeaveCriticalSection(&c->data.retr.cs);

			continue;
		}
		
		LeaveCriticalSection(&c->data.retr.cs);

		// get some data.
		if (ReadFile(c->data.retr.file,c->data.retr.buffer,ETP_SERVER_RETR_CHUNK_SIZE,&num_bytes_read,0))
		{
			if (num_bytes_read)
			{
				EnterCriticalSection(&c->data.retr.cs);
				c->data.retr.size = num_bytes_read;
				c->data.retr.remaining = num_bytes_read;
				LeaveCriticalSection(&c->data.retr.cs);
				
				// update data socket with new retr data.
				PostMessage(_etp_server->hwnd,ETP_SERVER_WM_DATA,c->data_socket,0);
			}
			else
			{
				// EOF
				EnterCriticalSection(&c->data.retr.cs);
				c->data.retr.state = 1;
				LeaveCriticalSection(&c->data.retr.cs);
				
				// update data socket with new retr data.
				PostMessage(_etp_server->hwnd,ETP_SERVER_WM_DATA,c->data_socket,0);

				break;			
			}
		}
		else
		{
			EnterCriticalSection(&c->data.retr.cs);
			c->data.retr.state = 2;
			LeaveCriticalSection(&c->data.retr.cs);
				
			// update data socket with new retr data.
			PostMessage(_etp_server->hwnd,ETP_SERVER_WM_DATA,c->data_socket,0);

			break;
		}
	}

	return 0;
}

static void etp_server_open_data(etp_server_client_t *c)
{
	if (c->data_socket == EVERYTHING_PLUGIN_OS_WINSOCK_INVALID_SOCKET)
	{
		switch (c->data_connection_type)
		{
			default:
				// nothing else will happen..
				etp_server_client_printf(c,(const everything_plugin_utf8_t *)"503 Invalid sequence of commands.\r\n");
				etp_server_close_data(c);
				break;

			case ETP_SERVER_CLIENT_DATA_CONNECTION_TYPE_PORT:

				// start connecting..
				// create the _etp_server socket
				c->data_socket = everything_plugin_os_winsock_socket(EVERYTHING_PLUGIN_OS_WINSOCK_AF_INET,EVERYTHING_PLUGIN_OS_WINSOCK_SOCK_STREAM,EVERYTHING_PLUGIN_OS_WINSOCK_IPPROTO_TCP);
				if (c->data_socket != EVERYTHING_PLUGIN_OS_WINSOCK_INVALID_SOCKET) 
				{
					struct everything_plugin_os_winsock_addrinfo *ai;
					struct everything_plugin_os_winsock_addrinfo hints;
					everything_plugin_utf8_buf_t port_cbuf;
					everything_plugin_utf8_buf_t host_cbuf;
				
					everything_plugin_utf8_buf_init(&host_cbuf);
					everything_plugin_utf8_buf_init(&port_cbuf);
					
					everything_plugin_network_set_tcp_nodelay(c->data_socket);
					everything_plugin_network_set_keepalive(c->data_socket);

					everything_plugin_os_winsock_WSAAsyncSelect(c->data_socket,_etp_server->hwnd,ETP_SERVER_WM_DATA,EVERYTHING_PLUGIN_OS_WINSOCK_FD_WRITE|EVERYTHING_PLUGIN_OS_WINSOCK_FD_READ|EVERYTHING_PLUGIN_OS_WINSOCK_FD_CLOSE|EVERYTHING_PLUGIN_OS_WINSOCK_FD_CONNECT);

					// Fill out the local socket address data.
					everything_plugin_os_zero_memory(&hints,sizeof(struct everything_plugin_os_winsock_addrinfo));
					hints.ai_family = EVERYTHING_PLUGIN_OS_WINSOCK_AF_INET;
					hints.ai_protocol = EVERYTHING_PLUGIN_OS_WINSOCK_IPPROTO_TCP;
					hints.ai_socktype = EVERYTHING_PLUGIN_OS_WINSOCK_SOCK_STREAM;
					
					everything_plugin_utf8_buf_printf(&host_cbuf,(const everything_plugin_utf8_t *)"%d.%d.%d.%d",c->data_connection_data.port.ip[0],c->data_connection_data.port.ip[1],c->data_connection_data.port.ip[2],c->data_connection_data.port.ip[3]);
					everything_plugin_utf8_buf_printf(&port_cbuf,(const everything_plugin_utf8_t *)"%d",c->data_connection_data.port.port);

					if (everything_plugin_os_winsock_getaddrinfo((char *)host_cbuf.buf,(char *)port_cbuf.buf,&hints,&ai) == 0)
					{
						if (everything_plugin_os_winsock_connect(c->data_socket,ai->ai_addr,(int)ai->ai_addrlen) != EVERYTHING_PLUGIN_OS_WINSOCK_SOCKET_ERROR) 
						{
							// do list.
							etp_server_run_data_command(c);
						}
						else
						{
							if (everything_plugin_os_winsock_WSAGetLastError() == WSAEWOULDBLOCK)
							{
								// wait for connection..
							}
							else
							{
								etp_server_client_printf(c,(const everything_plugin_utf8_t *)"500 Failed to connect to port %d.\r\n",everything_plugin_os_winsock_WSAGetLastError());

								etp_server_close_data(c);
							}
						}
						
						everything_plugin_os_winsock_freeaddrinfo(ai);
					}
					else
					{
						etp_server_client_printf(c,(const everything_plugin_utf8_t *)"500 Failed to get port address.\r\n");
						
						etp_server_close_data(c);
					}
					
					everything_plugin_utf8_buf_kill(&port_cbuf);
					everything_plugin_utf8_buf_kill(&host_cbuf);
				}
				else
				{
					etp_server_client_printf(c,(const everything_plugin_utf8_t *)"500 Failed to create port socket %d.\r\n",everything_plugin_os_winsock_WSAGetLastError());

					etp_server_close_data(c);
				}
				
				break;
				
			case ETP_SERVER_CLIENT_DATA_CONNECTION_TYPE_EPRT:
			
				c->data_socket = everything_plugin_os_winsock_socket(c->data_connection_data.eprt_addr.ss_family,EVERYTHING_PLUGIN_OS_WINSOCK_SOCK_STREAM,EVERYTHING_PLUGIN_OS_WINSOCK_IPPROTO_TCP);
				if (c->data_socket != EVERYTHING_PLUGIN_OS_WINSOCK_INVALID_SOCKET) 
				{
					everything_plugin_network_set_tcp_nodelay(c->data_socket);
					everything_plugin_network_set_keepalive(c->data_socket);

					everything_plugin_os_winsock_WSAAsyncSelect(c->data_socket,_etp_server->hwnd,ETP_SERVER_WM_DATA,EVERYTHING_PLUGIN_OS_WINSOCK_FD_WRITE|EVERYTHING_PLUGIN_OS_WINSOCK_FD_READ|EVERYTHING_PLUGIN_OS_WINSOCK_FD_CLOSE|EVERYTHING_PLUGIN_OS_WINSOCK_FD_CONNECT);

		//DEBUG:
/*
{
	everything_plugin_utf8_buf_t host_buf;
	
	everything_plugin_utf8_buf_init(&host_buf);

	if (c->data_connection_data.eprt_addr.ss_family == OS_WINSOCK_AF_INET)
	{
		everything_plugin_utf8_buf_format_sockaddr_in(&host_buf,(struct os_sockaddr_in *)&c->data_connection_data.eprt_addr);
	}
	else
	{
		everything_plugin_utf8_buf_format_struct everything_plugin_os_winsock_sockaddr_in6(&host_buf,(struct everything_plugin_os_winsock_sockaddr_in6 *)&c->data_connection_data.eprt_addr);
	}

	everything_plugin_debug_printf((const everything_plugin_utf8_t *)"connect to port |%d|%s|%d|\n",c->data_connection_data.eprt_addr.ss_family == OS_WINSOCK_AF_INET ? 1 : 2,host_buf.buf,c->data_connection_data.eprt_addr.ss_family == OS_WINSOCK_AF_INET ? ((struct os_sockaddr_in *)&c->data_connection_data.eprt_addr)->sin_port : ((struct everything_plugin_os_winsock_sockaddr_in6 *)&c->data_connection_data.eprt_addr)->sin6_port);

	everything_plugin_utf8_buf_kill(&host_buf);
}
*/
					if (everything_plugin_os_winsock_connect(c->data_socket,(struct everything_plugin_os_winsock_sockaddr *)&c->data_connection_data.eprt_addr,c->data_connection_data.eprt_addr.ss_family == EVERYTHING_PLUGIN_OS_WINSOCK_AF_INET ? sizeof(struct everything_plugin_os_sockaddr_in) : sizeof(struct everything_plugin_os_winsock_sockaddr_in6)) != EVERYTHING_PLUGIN_OS_WINSOCK_SOCKET_ERROR) 
					{
						// do list.
						etp_server_run_data_command(c);
					}
					else
					{
						if (everything_plugin_os_winsock_WSAGetLastError() == WSAEWOULDBLOCK)
						{
							// wait for connection..
						}
						else
						{
							etp_server_client_printf(c,(const everything_plugin_utf8_t *)"500 Failed to connect to port %d.\r\n",everything_plugin_os_winsock_WSAGetLastError());

							etp_server_close_data(c);
						}
					}
				}
				else
				{
					etp_server_client_printf(c,(const everything_plugin_utf8_t *)"500 Failed to create port socket %d.\r\n",everything_plugin_os_winsock_WSAGetLastError());

					etp_server_close_data(c);
				}
				
				break;
				
			case ETP_SERVER_CLIENT_DATA_CONNECTION_TYPE_PASV:
				
				break;
				
		}
	}
	else
	{
		etp_server_run_data_command(c);
	}
}

static void etp_server_run_data_command(etp_server_client_t *c)
{
	c->data_is_connected = 1;

	// command can be zero, in which case
	// we leave the data socket open and wait for the command..
	switch(c->data_type)
	{
		case ETP_SERVER_CLIENT_DATA_TYPE_LIST:
		{
			everything_plugin_db_find_t *db_find;
			everything_plugin_fileinfo_fd_t fd;
			static const char *months[12] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
			everything_plugin_utf8_buf_t filename_cbuf;
			SYSTEMTIME current_time;
			int current_month;
			
			everything_plugin_utf8_buf_init(&filename_cbuf);
			
			etp_server_client_printf(c,(const everything_plugin_utf8_t *)"150 Data connection accepted; transfer starting.\r\n");
			
			GetLocalTime(&current_time);
			current_month = current_time.wYear * 12 + (current_time.wMonth - 1);
			
			db_find = everything_plugin_db_find_first_file(_etp_server->db,c->data.list.path,&filename_cbuf,&fd);
			
			if (db_find)
			{
				everything_plugin_utf8_buf_t year_or_time_cbuf;
				
				everything_plugin_utf8_buf_init(&year_or_time_cbuf);

				for(;;)
				{
					SYSTEMTIME localst;
					int filemonth;
					int totfilemonth;

					everything_plugin_os_filetime_to_localtime(&localst,fd.date_modified);
					
					if (localst.wYear < 1900)
					{
						localst.wYear = 1900;
					}

					filemonth = localst.wMonth - 1;
					if (!((filemonth >= 0) && (filemonth < 12)))
					{
						filemonth = 0;
					}
					
					totfilemonth = (localst.wYear * 12) + filemonth;

					if (totfilemonth - filemonth <= 6)
					{
						everything_plugin_utf8_buf_printf(&year_or_time_cbuf,(const everything_plugin_utf8_t *)"%02d:%02d",localst.wHour,localst.wMinute);
					}
					else
					{
						everything_plugin_utf8_buf_printf(&year_or_time_cbuf,(const everything_plugin_utf8_t *)"%5d",localst.wYear);
					}
					
					//ftp ftp 3210987654321
					//ftp ftp           213
					etp_server_client_A(c,(const everything_plugin_utf8_t *)"%cr--r--r-- 1 ftp ftp %13I64u %s %02d %s %s\r\n",(fd.attributes & FILE_ATTRIBUTE_DIRECTORY) ? 'd' : '-',fd.size != EVERYTHING_PLUGIN_QWORD_MAX ? fd.size : 0,months[filemonth],localst.wDay,year_or_time_cbuf.buf,filename_cbuf.buf);

					if (!everything_plugin_db_find_next_file(db_find,&filename_cbuf,&fd)) 
					{
						break;
					}
				}
				
				// close find
				everything_plugin_db_find_close(db_find);

				everything_plugin_utf8_buf_kill(&year_or_time_cbuf);
			}

			// start processing data.		
			PostMessage(_etp_server->hwnd,ETP_SERVER_WM_DATA,c->data_socket,0);

			everything_plugin_utf8_buf_kill(&filename_cbuf);

			break;
		}

		case ETP_SERVER_CLIENT_DATA_TYPE_MLSD:
		{
			everything_plugin_db_find_t *db_find;
			everything_plugin_fileinfo_fd_t fd;
			everything_plugin_utf8_buf_t filename_cbuf;
			
			everything_plugin_utf8_buf_init(&filename_cbuf);
			
			etp_server_client_printf(c,(const everything_plugin_utf8_t *)"150 Data connection accepted; transfer starting.\r\n");
			
			// always use utf-8 for mlsd.
			db_find = everything_plugin_db_find_first_file(_etp_server->db,c->data.list.path,&filename_cbuf,&fd);
			
			if (db_find)
			{
				for(;;)
				{
					etp_server_client_UTF8(c,(const everything_plugin_utf8_t *)"type=%s;",(fd.attributes & FILE_ATTRIBUTE_DIRECTORY) ? (const everything_plugin_utf8_t *)"dir" : (const everything_plugin_utf8_t *)"file");
					
					if (fd.date_modified != EVERYTHING_PLUGIN_QWORD_MAX)
					{
						SYSTEMTIME st;
						
						// FTP uses UTC.
						// filetimes are already in UTC.
						FileTimeToSystemTime((FILETIME *)&fd.date_modified,&st);
						
						etp_server_client_UTF8(c,(const everything_plugin_utf8_t *)"modify=%04d%02d%02d%02d%02d%02d;",st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond);
					}
					
					if (fd.size != EVERYTHING_PLUGIN_QWORD_MAX)
					{
						etp_server_client_UTF8(c,(const everything_plugin_utf8_t *)"size=%I64u;",fd.size);
					}
					
					etp_server_client_UTF8(c,(const everything_plugin_utf8_t *)" %s\r\n",filename_cbuf.buf);

					if (!everything_plugin_db_find_next_file(db_find,&filename_cbuf,&fd)) 
					{
						break;
					}
				}
				
				// close find
				everything_plugin_db_find_close(db_find);
			}

			// start processing data.		
			PostMessage(_etp_server->hwnd,ETP_SERVER_WM_DATA,c->data_socket,0);

			everything_plugin_utf8_buf_kill(&filename_cbuf);

			break;
		}

		case ETP_SERVER_CLIENT_DATA_TYPE_RETR:

			etp_server_client_printf(c,(const everything_plugin_utf8_t *)"150 Data connection accepted; transfer starting.\r\n");

			// start processing data.		
			PostMessage(_etp_server->hwnd,ETP_SERVER_WM_DATA,c->data_socket,0);
					
			break;
	}
}

static void etp_server_send_query_results(etp_server_client_t *c)
{
	uintptr_t count;
	DWORD index;
	DWORD show_max;
	everything_plugin_utf8_buf_t name_cbuf;
	everything_plugin_utf8_buf_t path_cbuf;
	int is_index_folder_size;

	everything_plugin_utf8_buf_init(&name_cbuf);
	everything_plugin_utf8_buf_init(&path_cbuf);

	count = everything_plugin_db_query_get_result_count(c->db_query);
	index = c->offset;
	show_max = c->count;
	is_index_folder_size = everything_plugin_db_is_index_folder_size(_etp_server->db);

	etp_server_client_printf(c,(const everything_plugin_utf8_t *)"200-Query results\r\n",everything_plugin_db_query_get_result_count(c->db_query));
	etp_server_client_printf(c,(const everything_plugin_utf8_t *)" RESULT_COUNT %zu\r\n",everything_plugin_db_query_get_result_count(c->db_query));
	
	while(index < count)
	{
		everything_plugin_fileinfo_fd_t fd;
		int is_folder;
		
		if (!show_max)
		{
			break;
		}
		
		everything_plugin_db_query_get_result_name(c->db_query,index,&name_cbuf);
		everything_plugin_db_query_get_result_path(c->db_query,index,&path_cbuf);
		is_folder = everything_plugin_db_query_is_folder_result(c->db_query,index);

		if (c->current_path_column)
		{
			etp_server_client_printf(c,(const everything_plugin_utf8_t *)" PATH %s\r\n",path_cbuf.buf);
		}
		
		// get fd?
		if ((c->current_size_column) || (c->current_attributes_column) || (c->current_date_modified_column) || (c->current_date_created_column))
		{
			everything_plugin_db_query_get_result_indexed_fd(c->db_query,index,&fd);
			
			if (c->current_attributes_column)
			{
				etp_server_client_printf(c,(const everything_plugin_utf8_t *)" ATTRIBUTES %u\r\n",fd.attributes);
			}
			
			if (c->current_size_column)
			{
				if ((!is_folder) || (is_index_folder_size))
				{
					etp_server_client_printf(c,(const everything_plugin_utf8_t *)" SIZE %I64u\r\n",fd.size);
				}
				else
				{
					etp_server_client_printf(c,(const everything_plugin_utf8_t *)" SIZE %I64u\r\n",EVERYTHING_PLUGIN_QWORD_MAX);
				}
			}
			
			if (c->current_date_modified_column)
			{
				etp_server_client_printf(c,(const everything_plugin_utf8_t *)" DATE_MODIFIED %I64u\r\n",fd.date_modified);
			}
			
			if (c->current_date_created_column)
			{
				etp_server_client_printf(c,(const everything_plugin_utf8_t *)" DATE_CREATED %I64u\r\n",fd.date_created);
			}
		}

		if (c->current_file_list_filename_column)
		{
			everything_plugin_utf8_buf_t file_list_filename_cbuf;
			
			everything_plugin_utf8_buf_init(&file_list_filename_cbuf);
			
			everything_plugin_db_query_get_result_file_list_filename(c->db_query,index,&file_list_filename_cbuf);
			
			etp_server_client_printf(c,(const everything_plugin_utf8_t *)" FILE_LIST_FILENAME %s\r\n",file_list_filename_cbuf.buf);

			everything_plugin_utf8_buf_kill(&file_list_filename_cbuf);
		}
		
		if (c->current_date_recently_changed_column)
		{
			etp_server_client_printf(c,(const everything_plugin_utf8_t *)" DATE_RECENTLY_CHANGED %I64u\r\n",everything_plugin_db_query_get_result_date_recently_changed(c->db_query,index));
		}
		
		// what we want:
		// wether its a file or folder? even though we have sent attributes.
		// specify total number of results and possibly selection count
		// 
		etp_server_client_printf(c,(const everything_plugin_utf8_t *)" %s %s\r\n",is_folder ? (const everything_plugin_utf8_t *)"FOLDER" : (const everything_plugin_utf8_t *)"FILE",name_cbuf.buf);
		
		index++;
		show_max--;
	}
	
	etp_server_client_printf(c,(const everything_plugin_utf8_t *)"200 End.\r\n",everything_plugin_db_query_get_result_count(c->db_query));

	everything_plugin_utf8_buf_kill(&path_cbuf);
	everything_plugin_utf8_buf_kill(&name_cbuf);
}

//DEBUG_FIXME("OPTIMIZE")
static const everything_plugin_property_t *etp_server_get_sort_property_from_name(const everything_plugin_utf8_t *sort_name)
{
	int i;
	
	for(i=0;i<ETP_SERVER_NUM_SORT_NAME_TO_IDS;i++)
	{
		if (everything_plugin_utf8_string_compare_nocase_s_sla(sort_name,etp_server_sort_name_to_ids[i].name) == 0)
		{
			return everything_plugin_property_get_builtin_type(etp_server_sort_name_to_ids[i].property_type);
		}
	}
	
	return 0;
};

//DEBUG_FIXME("OPTIMIZE")
static int etp_server_get_sort_ascending_from_name(const everything_plugin_utf8_t *sort_name)
{
	int i;
	
	for(i=0;i<ETP_SERVER_NUM_SORT_NAME_TO_IDS;i++)
	{
		if (everything_plugin_utf8_string_compare_nocase_s_sla(sort_name,etp_server_sort_name_to_ids[i].name) == 0)
		{
			return etp_server_sort_name_to_ids[i].ascending;
		}
	}
	
	return 0;
};

static void etp_server_update_options_page(HWND page_hwnd)
{
	int is_enabled;
	int is_logging_enabled;
	
	is_enabled = (IsDlgButtonChecked(page_hwnd,ETP_SERVER_PLUGIN_ID_ENABLED_CHECKBOX) == BST_CHECKED) ? 1 : 0;
	is_logging_enabled = 0;
	
	if ((is_enabled) && (IsDlgButtonChecked(page_hwnd,ETP_SERVER_PLUGIN_ID_LOGGING_ENABLED_CHECKBOX) == BST_CHECKED))
	{
		is_logging_enabled = 1;
	}

	everything_plugin_os_enable_or_disable_dlg_item(page_hwnd,ETP_SERVER_PLUGIN_ID_LOGGING_ENABLED_CHECKBOX,is_enabled);
	everything_plugin_os_enable_or_disable_dlg_item(page_hwnd,ETP_SERVER_PLUGIN_ID_ALLOW_FILE_DOWNLOAD_CHECKBOX,is_enabled);
	everything_plugin_os_enable_or_disable_dlg_item(page_hwnd,ETP_SERVER_PLUGIN_ID_PORT_STATIC,is_enabled);
	everything_plugin_os_enable_or_disable_dlg_item(page_hwnd,ETP_SERVER_PLUGIN_ID_PORT_EDIT,is_enabled);
	everything_plugin_os_enable_or_disable_dlg_item(page_hwnd,ETP_SERVER_PLUGIN_ID_USERNAME_STATIC,is_enabled);
	everything_plugin_os_enable_or_disable_dlg_item(page_hwnd,ETP_SERVER_PLUGIN_ID_USERNAME_EDIT,is_enabled);
	everything_plugin_os_enable_or_disable_dlg_item(page_hwnd,ETP_SERVER_PLUGIN_ID_PASSWORD_STATIC,is_enabled);
	everything_plugin_os_enable_or_disable_dlg_item(page_hwnd,ETP_SERVER_PLUGIN_ID_PASSWORD_EDIT,is_enabled);
	everything_plugin_os_enable_or_disable_dlg_item(page_hwnd,ETP_SERVER_PLUGIN_ID_BINDINGS_STATIC,is_enabled);
	everything_plugin_os_enable_or_disable_dlg_item(page_hwnd,ETP_SERVER_PLUGIN_ID_BINDINGS_EDIT,is_enabled);

	everything_plugin_os_enable_or_disable_dlg_item(page_hwnd,ETP_SERVER_PLUGIN_ID_LOG_FILE_NAME_EDIT,is_logging_enabled);
	everything_plugin_os_enable_or_disable_dlg_item(page_hwnd,ETP_SERVER_PLUGIN_ID_LOG_FILE_NAME_BROWSE_BUTTON,is_logging_enabled);
	everything_plugin_os_enable_or_disable_dlg_item(page_hwnd,ETP_SERVER_PLUGIN_ID_LOG_MAX_SIZE_EDIT,is_logging_enabled);
	everything_plugin_os_enable_or_disable_dlg_item(page_hwnd,ETP_SERVER_PLUGIN_ID_LOG_FILE_STATIC,is_logging_enabled);
	everything_plugin_os_enable_or_disable_dlg_item(page_hwnd,ETP_SERVER_PLUGIN_ID_MAX_SIZE_STATIC,is_logging_enabled);
	everything_plugin_os_enable_or_disable_dlg_item(page_hwnd,ETP_SERVER_PLUGIN_ID_KB_STATIC,is_logging_enabled);
}

static void etp_server_create_checkbox(everything_plugin_load_options_page_t *load_options_page,int id,DWORD extra_style,int text_localization_id,int tooltip_localization_id,int checked)
{
	everything_plugin_os_create_checkbox(load_options_page->page_hwnd,id,extra_style,checked,everything_plugin_localization_get_string(text_localization_id));

	everything_plugin_os_add_tooltip(load_options_page->tooltip_hwnd,load_options_page->page_hwnd,id,everything_plugin_localization_get_string(tooltip_localization_id));
}

static void etp_server_create_static(everything_plugin_load_options_page_t *load_options_page,int id,int text_localization_id)
{
	everything_plugin_os_create_static(load_options_page->page_hwnd,id,SS_LEFTNOWORDWRAP|WS_GROUP,everything_plugin_localization_get_string(text_localization_id));
}

static void etp_server_create_edit(everything_plugin_load_options_page_t *load_options_page,int id,int tooltip_localization_id,const everything_plugin_utf8_t *text)
{
	everything_plugin_os_create_edit(load_options_page->page_hwnd,id,WS_GROUP,text);

	everything_plugin_os_add_tooltip(load_options_page->tooltip_hwnd,load_options_page->page_hwnd,id,everything_plugin_localization_get_string(tooltip_localization_id));
}

static void etp_server_create_number_edit(everything_plugin_load_options_page_t *load_options_page,int id,int tooltip_localization_id,int value)
{
	everything_plugin_os_create_number_edit(load_options_page->page_hwnd,id,WS_GROUP,value);

	everything_plugin_os_add_tooltip(load_options_page->tooltip_hwnd,load_options_page->page_hwnd,id,everything_plugin_localization_get_string(tooltip_localization_id));
}

static void etp_server_create_password_edit(everything_plugin_load_options_page_t *load_options_page,int id,int tooltip_localization_id,const everything_plugin_utf8_t *text)
{
	everything_plugin_os_create_password_edit(load_options_page->page_hwnd,id,WS_GROUP,text);

	everything_plugin_os_add_tooltip(load_options_page->tooltip_hwnd,load_options_page->page_hwnd,id,everything_plugin_localization_get_string(tooltip_localization_id));
}

static void etp_server_create_button(everything_plugin_load_options_page_t *load_options_page,int id,DWORD extra_style,int text_localization_id,int tooltip_localization_id)
{
	everything_plugin_os_create_button(load_options_page->page_hwnd,id,extra_style,everything_plugin_localization_get_string(text_localization_id));

	everything_plugin_os_add_tooltip(load_options_page->tooltip_hwnd,load_options_page->page_hwnd,id,everything_plugin_localization_get_string(tooltip_localization_id));
}

static void etp_server_enable_options_apply(everything_plugin_options_page_proc_t *options_page_proc)
{
	everything_plugin_os_enable_or_disable_dlg_item(options_page_proc->options_hwnd,1001,1);
}

static int etp_server_expand_min_wide(HWND page_hwnd,int text_localization_id,int current_wide)
{
	int wide;
	
	wide = everything_plugin_os_expand_dialog_text_logical_wide_no_prefix(page_hwnd,everything_plugin_localization_get_string(text_localization_id),current_wide);
	
	if (wide > current_wide)
	{
		return wide;
	}
	
	return current_wide;
}

static everything_plugin_utf8_t *etp_server_get_options_text(HWND page_hwnd,int id,everything_plugin_utf8_t *old_value)
{
	everything_plugin_utf8_buf_t cbuf;
	everything_plugin_utf8_t *ret;

	everything_plugin_utf8_buf_init(&cbuf);
	
	everything_plugin_os_get_dlg_text(page_hwnd,id,&cbuf);
	
	ret = everything_plugin_utf8_string_realloc_utf8_string(old_value,cbuf.buf);

	everything_plugin_utf8_buf_kill(&cbuf);
	
	return ret;
}

