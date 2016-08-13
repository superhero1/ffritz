<?lua
g_page_type = "all"
g_page_title = ""
g_page_help = "hilfe_internet_ipv6.html"
dofile("../templates/global_lua.lua")
require("val")
require("boxvars2")
require("elem")
ipv6_enabled = boxvars2.c_boxvars:init( { sz_query = "ipv6:settings/enabled"} ) -- ( 1 )
ipv6_mode = boxvars2.c_boxvars:init( { sz_query = "ipv6:settings/ipv6_mode"} ) -- ( 2 )
ipv6_state = boxvars2.c_boxvars:init( { sz_query = "ipv6:settings/state"} ) -- ( 3 )
box_state_localtime = boxvars2.c_boxvars:init( { sz_query = "box:status/localtime"} ) -- ( 4 )
ipv6_uptime = boxvars2.c_boxvars:init( { sz_query = "ipv6:settings/uptime"} ) -- ( 5 )
ipv6_state_connect_date = boxvars2.c_boxvars:init( { sz_query = "ipv6:status/conntime_date"} ) -- ( 6 )
ipv6_state_connect_time = boxvars2.c_boxvars:init( { sz_query = "ipv6:status/conntime_time"} ) -- ( 7 )
ipv6_ipadr = boxvars2.c_boxvars:init( { sz_query = "ipv6:settings/ip"} ) -- ( 8 )
ipv6_ipadr_valid = boxvars2.c_boxvars:init( { sz_query = "ipv6:settings/ip_valid"} ) -- ( 9 )
ipv6_ipadr_preferred = boxvars2.c_boxvars:init( { sz_query = "ipv6:settings/ip_preferred"} ) -- ( 10 )
ipv6_mtu = boxvars2.c_boxvars:init( { sz_query = "ipv6:settings/mtu"} ) -- ( 11 )
ipv6_prefix = boxvars2.c_boxvars:init( { sz_query = "ipv6:settings/prefix"} ) -- ( 12 )
ipv6_prefix_valid = boxvars2.c_boxvars:init( { sz_query = "ipv6:settings/prefix_valid"} ) -- ( 13 )
ipv6_prefix_preferred = boxvars2.c_boxvars:init( { sz_query = "ipv6:settings/prefix_preferred"} ) -- ( 14 )
ipv6_addr_learned = boxvars2.c_boxvars:init( { sz_query = "ipv6:settings/addr_learned"} ) -- ( 15 )
ipv6_first_dns = boxvars2.c_boxvars:init( { sz_query = "ipv6:settings/first_dns"} ) -- ( 16 )
ipv6_second_dns = boxvars2.c_boxvars:init( { sz_query = "ipv6:settings/second_dns"} ) -- ( 17 )
ipv6_static_prefix = boxvars2.c_boxvars:init( { sz_query = "ipv6:settings/static_prefix"} ) -- ( 40 )
ipv6_static_prefixlen = boxvars2.c_boxvars:init( { sz_query = "ipv6:settings/static_prefixlen"} ) -- ( 41 )
ipv6_wan_use_first_prefix = boxvars2.c_boxvars:init( { sz_query = "ipv6:settings/wan_use_first_prefix"} ) -- ( 42 )
ipv6_wan_prefix = boxvars2.c_boxvars:init( { sz_query = "ipv6:settings/wan_prefix"} ) -- ( 43 )
ipv6_wan_ifid_automatic = boxvars2.c_boxvars:init( { sz_query = "ipv6:settings/wan_ifid_automatic"} ) -- ( 44 )
ipv6_wan_ifid = boxvars2.c_boxvars:init( { sz_query = "ipv6:settings/wan_ifid"} ) -- ( 45 )
ipv6_wan_first_dns = boxvars2.c_boxvars:init( { sz_query = "ipv6:settings/wan_first_dns"} ) -- ( 46 )
ipv6_wan_second_dns = boxvars2.c_boxvars:init( { sz_query = "ipv6:settings/wan_second_dns"} ) -- ( 47 )
ipv6_sixxs_username = boxvars2.c_boxvars:init( { sz_query = "ipv6:settings/sixxs_username"} ) -- ( 18 )
ipv6_sixxs_passwd = boxvars2.c_boxvars:init( { sz_query = "ipv6:settings/sixxs_passwd"} ) -- ( 19 )
ipv6_sixxs_tunnelid = boxvars2.c_boxvars:init( { sz_query = "ipv6:settings/sixxs_tunnelid"} ) -- ( 20 )
ipv6_6rd_popaddr = boxvars2.c_boxvars:init( { sz_query = "ipv6:settings/6rd_popaddr"} ) -- ( 21 )
ipv6_6rd_prefix = boxvars2.c_boxvars:init( { sz_query = "ipv6:settings/6rd_prefix"} ) -- ( 22 )
ipv6_6rd_prefixlen = boxvars2.c_boxvars:init( { sz_query = "ipv6:settings/6rd_prefixlen"} ) -- ( 23 )
ipv6_6rd_ipv4masklen = boxvars2.c_boxvars:init( { sz_query = "ipv6:settings/6rd_ipv4masklen"} ) -- ( 39 )
ipv6_6to4static_popaddr = boxvars2.c_boxvars:init( { sz_query = "ipv6:settings/6to4static_popaddr"} ) -- ( 24 )
ipv6_6to4static_remote = boxvars2.c_boxvars:init( { sz_query = "ipv6:settings/6to4static_remote"} ) -- ( 25 )
ipv6_6to4static_local = boxvars2.c_boxvars:init( { sz_query = "ipv6:settings/6to4static_local"} ) -- ( 26 )
ipv6_6to4static_prefix = boxvars2.c_boxvars:init( { sz_query = "ipv6:settings/6to4static_prefix"} ) -- ( 27 )
ipv6_6to4static_prefixlen = boxvars2.c_boxvars:init( { sz_query = "ipv6:settings/6to4static_prefixlen"} ) -- ( 28 )
ipv6_use_fixed_mtu = boxvars2.c_boxvars:init( { sz_query = "ipv6:settings/use_fixed_mtu"} ) -- ( 29 )
ipv6_mtu_override = boxvars2.c_boxvars:init( { sz_query = "ipv6:settings/mtu_override"} ) -- ( 30 )
ipv6_ipv4_mode = boxvars2.c_boxvars:init( { sz_query = "ipv6:settings/ipv4_mode"} ) -- ( 36 )
ipv6_ipv4_active_mode = boxvars2.c_boxvars:init( { sz_query = "ipv6:settings/ipv4_active_mode"} ) -- ( 37 )
ipv6_aftr = boxvars2.c_boxvars:init( { sz_query = "ipv6:settings/ipv6_aftr"} ) -- ( 38 )
ipv6_active_aftr = boxvars2.c_boxvars:init( { sz_query = "ipv6:settings/ipv6_active_aftr"} ) -- ( 39 )
ipv6_active_rapid_commit = boxvars2.c_boxvars:init( { sz_query = "ipv6:settings/dhcpv6c_use_rapid_commit"} ) -- ( 40 )
ipv6_active_prefix = boxvars2.c_boxvars:init( { sz_query = "ipv6:settings/dhcpv6c_use_wanted_prefixlen"}) -- ( 41 )
ipv6_prefixlen = boxvars2.c_boxvars:init( { sz_query = "ipv6:settings/dhcpv6c_wanted_prefixlen"}) -- ( 41 )
ipv6_hidden_ipv4 = boxvars2.c_boxvars:init( { sz_query = "ipv6:settings/ipv4_hidden"} )
-- MOD
-- This entry in ar7.cfg is always overwritten at boot time ..
-- ipv6_hidden_ipv6_native = boxvars2.c_boxvars:init( { sz_query = "ipv6:settings/ipv6_native_hidden"} )
ipv6_hidden_ipv6_native = boxvars2.c_boxvars:init( { sz_query = "ipv6:settings/ipv6_hidden"} )
ipv6_hidden_ds_lite = boxvars2.c_boxvars:init( { sz_query = "ipv6:settings/ds_lite_hidden"} )
ipv6_hidden_gui = boxvars2.c_boxvars:init( { sz_query = "ipv6:settings/gui_hidden"} )
function wlan_guest_active()
if ( config.WLAN_GUEST ) then
local szWlanEnabled = box.query("wlan:settings/guest_ap_enabled")
if ( szWlanEnabled == "1") then
return false
else
return false
end
else
return false
end
end
function wlan_guest_no_active()
if ( config.WLAN_GUEST ) then
local szWlanEnabled = box.query("wlan:settings/guest_ap_enabled")
if ( szWlanEnabled == "1") then
return true
else
return true
end
else
return true
end
end
function is_ipv6_addr(str)
return val.is_ipv6(str)
end
function is_disabled(disabled)
if disabled then
return [[disabled = "disabled"]]
end
end
function is_aftr_disabled(elem)
local value = ipv6_aftr:get_value()
local is_ipv6_addr = is_ipv6_addr(value)
if elem == "fqdn" and is_ipv6_addr or elem == "ipv6" and not is_ipv6_addr then
return is_disabled(true)
end
end
function write_value(elem)
local value = ipv6_aftr:get_value()
local is_ipv6_addr = is_ipv6_addr(value)
if elem == "fqdn" and not is_ipv6_addr or elem == "ipv6" and is_ipv6_addr then
return value
end
return ""
end
g_val = {}
g_val.prog = [[]]
if (not general.is_ip_client()) then
g_val = {
prog = [[
if __checked(]]..ipv6_enabled:get_val_names()..[[) then
if __radio_check(]]..ipv6_mode:get_var_name_js()..[[_1/]]..ipv6_mode:get_var_name()..[[, 1) then
if __checked(]]..ipv6_ipv4_mode:get_val_names()..[[) then
if __radio_check(]]..ipv6_ipv4_mode:get_var_name_js()..[[_Static/]]..ipv6_ipv4_mode:get_var_name()..[[_static,ipv4_4to6static) then
if __radio_check(]]..ipv6_ipv4_mode:get_var_name_js()..[[_Static_IPV6/]]..ipv6_ipv4_mode:get_var_name()..[[_static_aftr,IPV6) then
native_prefix(]]..ipv6_aftr:get_var_name_js()..[[_IPV6/]]..ipv6_aftr:get_var_name()..[[_ipv6,native_prefix)
end
if __radio_check(]]..ipv6_ipv4_mode:get_var_name_js()..[[_Static_FQDN/]]..ipv6_ipv4_mode:get_var_name()..[[_static_aftr,FQDN) then
char_range_regex(]]..ipv6_aftr:get_var_name_js()..[[_FQDN/]]..ipv6_aftr:get_var_name()..[[_fqdn,fqdn,fqdn)
end
end
end
if __radio_check(ui_IPv6ModeNative_Static/native_mode,ipv6_static) then
native_prefix(]]..ipv6_static_prefix:get_val_names()..[[,native_prefix)
num_range(]]..ipv6_static_prefixlen:get_val_names()..[[, 1, 64, true, num_range_lan_len)
if __radio_check(]]..ipv6_wan_use_first_prefix:get_val_names([[_0]])..[[, 0) then
native_prefix(]]..ipv6_wan_prefix:get_val_names()..[[, native_prefix)
end
if __radio_check(]]..ipv6_wan_ifid_automatic:get_val_names([[_0]])..[[, 0) then
native_interface_id(]]..ipv6_wan_ifid:get_val_names()..[[, native_interface_id)
end
ipv6(]]..ipv6_wan_first_dns:get_val_names()..[[, ipv6)
ipv6(]]..ipv6_wan_second_dns:get_val_names()..[[, ipv6)
end
if __not_radio_check(ui_IPv6ModeNative_Static/native_mode,ipv6_static) then
if __checked(]]..ipv6_active_prefix:get_val_names()..[[) then
num_range(]]..ipv6_prefixlen:get_val_names()..[[,48, 64, num_range_lan_len)
end
end
end
if __radio_check(]]..ipv6_mode:get_var_name_js()..[[_2/]]..ipv6_mode:get_var_name()..[[,2) then
if __radio_check(ui_IPv6ModeTunnel_6rd/tunnel_mode,ipv6_6rd) then
num_range(]]..ipv6_6rd_ipv4masklen:get_val_names()..[[, 0, 32, true, num_range_mask)
num_range(]]..ipv6_6rd_prefixlen:get_val_names()..[[, 1, 64, true, num_range_prefix)
f6rd_prefixlen(]]..ipv6_6rd_ipv4masklen:get_val_names()..[[,]]..ipv6_6rd_prefixlen:get_val_names()..[[, t6rd_prefixlen_msg)
ipv4(ui_6rdEndPoint_/ipv4_6rd_, ipv4)
end
if __radio_check(ui_IPv6ModeTunnel_6to4static/tunnel_mode,ipv6_6to4static) then
if wlan_guest_active() then
num_range(]]..ipv6_6to4static_prefixlen:get_val_names()..[[, 1, 63, true, num_range_prefix)
end
if wlan_guest_no_active() then
num_range(]]..ipv6_6to4static_prefixlen:get_val_names()..[[, 1, 64, true, num_range_prefix)
end
ipv4(ui_6to4EndPoint_/ipv4_6to4_, ipv4)
end
end
if __checked(]]..ipv6_use_fixed_mtu:get_val_names()..[[) then
min_num(]]..ipv6_mtu_override:get_val_names()..[[, 1280, true, min_num)
end
end
]]
}
end
val.msg.native_prefix = {
[val.ret.empty] = [[{?683:153?}]],
[val.ret.format] = [[{?683:3733?}]],
[val.ret.wrong] = [[{?683:18?}]],
[val.ret.notfound] = [[{?683:364?}]]
}
val.msg.fqdn = {
[val.ret.empty] = [[{?683:436?}]],
[val.ret.format] = [[{?683:93?}]],
[val.ret.outofrange] = [[{?683:78?}]],
[val.ret.notfound] = [[{?683:362?}]]
}
val.msg.native_interface_id = {
[val.ret.empty] = [[{?683:609?}]],
[val.ret.format] = [[{?683:643?}]],
[val.ret.missing] = [[{?683:316?}]],
[val.ret.notfound] = [[{?683:454?}]]
}
val.msg.ipv6 = {
[val.ret.empty] = [[{?683:714?}]],
[val.ret.format] = [[{?683:569?}]],
[val.ret.wrong] = [[{?683:567?}]],
[val.ret.toomuch] = [[{?683:44?}]],
[val.ret.notfound] = [[{?683:16?}]]
}
val.msg.ipv4 = {
[val.ret.empty] = [[{?683:885?}]],
[val.ret.format] = [[{?683:197?}]],
[val.ret.outofrange] = [[{?683:628?}]],
[val.ret.outofnet] = [[{?683:238?}]],
[val.ret.thenet] = [[{?683:25?}]],
[val.ret.broadcast] = [[{?683:860?}]],
[val.ret.thebox] = [[{?683:774?}]],
[val.ret.unsized] = [[{?683:697?}]]
}
val.msg.num_range_prefix = {
[val.ret.notfound] = [[{?683:591?}]],
[val.ret.empty] = [[{?683:982?}]],
[val.ret.format] = [[{?683:31?}]],
[val.ret.outofrange] = [[{?683:9946?}]]
}
val.msg.num_range_mask= {
[val.ret.notfound] = [[{?683:750?}]],
[val.ret.empty] = [[{?683:558?}]],
[val.ret.format] = [[{?8718:9?}]],
[val.ret.outofrange] = [[{?683:713?}]]
}
val.msg.num_range_lan_len= {
[val.ret.notfound] = [[{?683:9063?}]],
[val.ret.empty] = [[{?683:997?}]],
[val.ret.format] = [[{?683:715?}]],
[val.ret.outofrange] = [[{?683:1713?}]]
}
val.msg.num_range_lan_len= {
[val.ret.notfound] = [[{?683:306?}]],
[val.ret.empty] = [[{?683:638?}]],
[val.ret.format] = [[{?683:9065?}]],
[val.ret.outofrange] = [[{?683:965?}]]
}
val.msg.t6rd_prefixlen_msg= {
[val.ret.outofrange] = [[{?683:676?}]]
}
val.msg.min_num= {
[val.ret.notfound] = [[{?683:14?}]],
[val.ret.empty] = [[{?683:793?}]],
[val.ret.format] = [[{?683:42?}]],
[val.ret.outofrange] = [[{?683:559?}]]
}
function write_dslite_tr_style(which)
local hide = false
hide = ipv6_ipv4_mode:get_value() == 'ipv4_normal'
if not hide and which == 'aftr' then
hide = ipv6_ipv4_active_mode:get_value() == 'ipv4_normal'
end
if hide then
box.out([[style="display:none;"]])
end
end
function write_dslite_active()
local active = ipv6_ipv4_active_mode:get_value() ~= 'ipv4_normal'
box.html(active and [[{?683:287?}]]
or [[{?683:735?}]]
)
end
function write_dslite_aftr()
box.html( ipv6_active_aftr:get_value())
end
function split(str, sep)
local result = {}
if not sep or sep == "" then
for i = 1, #str do
table.insert(result, str:sub(i,i))
end
return result
end
local curr = 1
local left, right = str:find(sep, curr, true)
while left do
table.insert(result, str:sub(curr, left-1))
curr = right + 1
left, right = str:find(sep, curr, true)
end
table.insert(result, str:sub(curr))
return result
end
function IpLearnedFrom( szIpLearned)
if ( szIpLearned == "ra") then
return [[{?683:299?}]]
elseif ( szIpLearned == "ia_pd") then
return [[{?683:877?}]]
elseif ( szIpLearned == "ia_na") then
return [[{?683:112?}]]
elseif ( szIpLearned == "6to4") then
return [[{?683:160?}]]
elseif ( szIpLearned == "6rd") then
return [[{?683:651?}]]
else
return [[{?683:810?}]]
end
end
function GetConnectMode( szMode)
g_IPv6_Display_Native = "none"
g_IPv6_Display_Tunnel = "none"
g_tConnectMode = { "", "", ""}
if ((szMode == "ipv6_tunnel") or (szMode == "ipv6_sixxs_heartbeat") or (szMode == "ipv6_6rd") or (szMode == "ipv6_6to4static")) then
g_tConnectMode[3] = "checked"
g_IPv6_Display_Tunnel = ""
elseif ((szMode == "ipv6_native") or (szMode == "ipv6_cable") or ( szMode == "ipv6_native_unnumbered")or ( szMode == "ipv6_static")) then
g_tConnectMode[2] = "checked"
g_IPv6_Display_Native = ""
elseif (szMode == "ipv6_automatic" or szMode == "ipv6_lte") then
g_tConnectMode[1] = "checked"
end
end
function IsIPv6NativeMode( szMode)
if ((szMode == "ipv6_native") or (szMode == "ipv6_cable") or ( szMode == "ipv6_native_unnumbered")or ( szMode == "ipv6_static")) then
return true
end
return false
end
function IsIPv6TunnelMode( szMode)
if ((szMode == "ipv6_tunnel") or (szMode == "ipv6_sixxs_heartbeat") or (szMode == "ipv6_6rd") or (szMode == "ipv6_6to4static")) then
return true
end
return false
end
function IsCurrentIPv6Mode( szMode, szValue)
if (szMode == szValue) then
return true
end
return false
end
function saveMutUlaPart(_t_save_set)
ipv6_use_fixed_mtu:save_check_value( _t_save_set)
if ( ipv6_use_fixed_mtu:var_exist()) then
ipv6_mtu_override:save_value( _t_save_set)
end
end
function rewrite_DS_Lite()
if ( ipv6_hidden_ds_lite:get_value() ~= "1") then
if ( ipv6_ipv4_mode:var_exist()) then
if ( box.post[ipv6_ipv4_mode:get_var_name()..[[auto]] ] ~= nil) then
ipv6_ipv4_mode:set_value( 'ipv4_ds_lite');
end
if ( box.post[ipv6_ipv4_mode:get_var_name()..[[_static]] ] ~= nil) then
ipv6_ipv4_mode:set_value( 'ipv4_4to6static');
ipv6_aftr:set_value( box.post[ipv6_aftr:get_var_name()])
end
else
ipv6_ipv4_mode:set_value( 'ipv4_normal');
end
end
end
function save_DS_Lite( _t_save_set)
if ( ipv6_hidden_ds_lite:get_value() ~= "1") then
if ( ipv6_ipv4_mode:var_exist()) then
if ( box.post[ipv6_ipv4_mode:get_var_name()..[[_auto]] ] ~= nil) then
cmtable.add_var( _t_save_set, ipv6_ipv4_mode:get_query_str(), 'ipv4_ds_lite')
ipv6_ipv4_mode:set_value( 'ipv4_ds_lite');
end
if ( box.post[ipv6_ipv4_mode:get_var_name()..[[_static]] ] ~= nil) then
cmtable.add_var( _t_save_set, ipv6_ipv4_mode:get_query_str(), 'ipv4_4to6static')
local aftr = box.post[ipv6_aftr:get_var_name()..[[_ipv6]]]
if ( box.post[ipv6_ipv4_mode:get_var_name()..[[_static_aftr]] ] == "FQDN") then
aftr = box.post[ipv6_aftr:get_var_name()..[[_fqdn]]]
end
ipv6_ipv4_mode:set_value( 'ipv4_4to6static');
ipv6_aftr:save_value( _t_save_set, aftr)
end
else
cmtable.add_var( _t_save_set, ipv6_ipv4_mode:get_query_str(), 'ipv4_normal')
ipv6_ipv4_mode:set_value( 'ipv4_normal');
end
end
end
function rewriteIPv6NativeStatics()
if ( ipv6_static_prefix:var_exist()) then
ipv6_static_prefix:set_value( box.post[ipv6_static_prefix:get_var_name()])
end
if ( ipv6_static_prefixlen:var_exist()) then
ipv6_static_prefixlen:set_value( box.post[ipv6_static_prefixlen:get_var_name()])
end
if ( ipv6_wan_use_first_prefix:var_exist()) then
ipv6_wan_use_first_prefix:set_value( box.post[ipv6_wan_use_first_prefix:get_var_name()])
end
if ( ipv6_wan_use_first_prefix:get_value() == "0") and ( ipv6_wan_prefix:var_exist()) then
ipv6_wan_prefix:set_value( box.post[ipv6_wan_prefix:get_var_name()])
end
if ( ipv6_wan_ifid_automatic:var_exist()) then
ipv6_wan_ifid_automatic:set_value( box.post[ipv6_wan_ifid_automatic:get_var_name()])
end
if ( ipv6_wan_ifid_automatic:get_value() == "0") and ( ipv6_wan_ifid:var_exist()) then
ipv6_wan_ifid:set_value( box.post[ipv6_wan_ifid:get_var_name()])
end
if ( ipv6_wan_first_dns:var_exist()) then
ipv6_wan_first_dns:set_value( box.post[ipv6_wan_first_dns:get_var_name()])
end
if ( ipv6_wan_second_dns:var_exist()) then
ipv6_wan_second_dns:set_value( box.post[ipv6_wan_second_dns:get_var_name()])
end
end
function saveIPv6NativeStatics( _t_save_set)
if ( ipv6_static_prefix:var_exist()) then
ipv6_static_prefix:save_value( _t_save_set)
end
if ( ipv6_static_prefixlen:var_exist()) then
ipv6_static_prefixlen:save_value( _t_save_set)
end
if ( ipv6_wan_use_first_prefix:var_exist()) then
ipv6_wan_use_first_prefix:save_value( _t_save_set)
end
if ( ipv6_wan_use_first_prefix:get_value() == "0") and ( ipv6_wan_prefix:var_exist()) then
ipv6_wan_prefix:save_value( _t_save_set)
end
if ( ipv6_wan_ifid_automatic:var_exist()) then
ipv6_wan_ifid_automatic:save_value( _t_save_set)
end
if ( ipv6_wan_ifid_automatic:get_value() == "0") and ( ipv6_wan_ifid:var_exist()) then
ipv6_wan_ifid:save_value( _t_save_set)
end
if ( ipv6_wan_first_dns:var_exist()) then
ipv6_wan_first_dns:save_value( _t_save_set)
end
if ( ipv6_wan_second_dns:var_exist()) then
ipv6_wan_second_dns:save_value( _t_save_set)
end
end
function rewriteIPv6TunnelSixxs()
if ( ipv6_sixxs_username:var_exist()) then
ipv6_sixxs_username:set_value( box.post[ipv6_sixxs_username:get_var_name()])
end
if ( ipv6_sixxs_passwd:var_exist()) then
ipv6_sixxs_passwd:set_value( box.post[ipv6_sixxs_passwd:get_var_name()])
end
if ( ipv6_sixxs_tunnelid:var_exist()) then
ipv6_sixxs_tunnelid:set_value( box.post[ipv6_sixxs_tunnelid:get_var_name()])
end
end
function saveIPv6TunnelSixxs(_t_save_set)
if ( ipv6_sixxs_username:var_exist()) then
ipv6_sixxs_username:save_value( _t_save_set)
end
if ( ipv6_sixxs_passwd:var_exist()) then
ipv6_sixxs_passwd:save_value( _t_save_set)
end
if ( ipv6_sixxs_tunnelid:var_exist()) then
ipv6_sixxs_tunnelid:save_value( _t_save_set)
end
end
function rewriteIPv6Tunnel6rd()
if ( box.post.ipv4_6rd_0 ~= nil) then
local l_szTemp = tostring(box.post.ipv4_6rd_0).."."..tostring(box.post.ipv4_6rd_1).."."..tostring(box.post.ipv4_6rd_2).."."..tostring(box.post.ipv4_6rd_3)
ipv6_6rd_popaddr:set_value( l_szTemp)
end
if ( ipv6_6rd_prefix:var_exist()) then
ipv6_6rd_prefix:set_value( box.post[ipv6_6rd_prefix:get_var_name()])
end
if ( ipv6_6rd_prefixlen:var_exist()) then
ipv6_6rd_prefixlen:set_value( box.post[ipv6_6rd_prefixlen:get_var_name()])
end
if ( ipv6_6rd_ipv4masklen:var_exist()) then
ipv6_6rd_ipv4masklen:set_value( box.post[ipv6_6rd_ipv4masklen:get_var_name()])
end
end
function saveIPv6Tunnel6rd(_t_save_set)
if ( box.post.ipv4_6rd_0 ~= nil) then
local l_szTemp = tostring(box.post.ipv4_6rd_0).."."..tostring(box.post.ipv4_6rd_1).."."..tostring(box.post.ipv4_6rd_2).."."..tostring(box.post.ipv4_6rd_3)
ipv6_6rd_popaddr:save_value( _t_save_set, l_szTemp)
end
if ( ipv6_6rd_prefix:var_exist()) then
ipv6_6rd_prefix:save_value( _t_save_set)
end
if ( ipv6_6rd_prefixlen:var_exist()) then
ipv6_6rd_prefixlen:save_value( _t_save_set)
end
if ( ipv6_6rd_ipv4masklen:var_exist()) then
ipv6_6rd_ipv4masklen:save_value( _t_save_set)
end
end
function rewriteIPv6Tunnel6to4()
if ( box.post.ipv4_6to4_0 ~= nil) then
local l_szTemp = tostring(box.post.ipv4_6to4_0).."."..tostring(box.post.ipv4_6to4_1).."."..tostring(box.post.ipv4_6to4_2).."."..tostring(box.post.ipv4_6to4_3)
ipv6_6to4static_popaddr:set_value( l_szTemp)
end
if ( ipv6_6to4static_remote:var_exist()) then
ipv6_6to4static_remote:set_value( box.post[ipv6_6to4static_remote:get_var_name()])
end
if ( ipv6_6to4static_local:var_exist()) then
ipv6_6to4static_local:set_value( box.post[ipv6_6to4static_local:get_var_name()])
end
if ( ipv6_6to4static_prefix:var_exist()) then
ipv6_6to4static_prefix:set_value( box.post[ipv6_6to4static_prefix:get_var_name()])
end
if ( ipv6_6to4static_prefixlen:var_exist()) then
ipv6_6to4static_prefixlen:set_value( box.post[ipv6_6to4static_prefixlen:get_var_name()])
end
end
function saveIPv6Tunnel6to4(_t_save_set)
if ( box.post.ipv4_6to4_0 ~= nil) then
local l_szTemp = tostring(box.post.ipv4_6to4_0).."."..tostring(box.post.ipv4_6to4_1).."."..tostring(box.post.ipv4_6to4_2).."."..tostring(box.post.ipv4_6to4_3)
ipv6_6to4static_popaddr:save_value(_t_save_set, l_szTemp)
end
if ( ipv6_6to4static_remote:var_exist()) then
ipv6_6to4static_remote:save_value(_t_save_set)
end
if ( ipv6_6to4static_local:var_exist()) then
ipv6_6to4static_local:save_value(_t_save_set)
end
if ( ipv6_6to4static_prefix:var_exist()) then
ipv6_6to4static_prefix:save_value(_t_save_set)
end
if ( ipv6_6to4static_prefixlen:var_exist()) then
ipv6_6to4static_prefixlen:save_value( _t_save_set)
end
end
function rewriteMutUlaPart()
if ( ipv6_use_fixed_mtu:var_exist()) then
ipv6_use_fixed_mtu:set_value( "1")
ipv6_mtu_override:set_value( box.post[ipv6_mtu_override:get_var_name()])
else
ipv6_use_fixed_mtu:set_value( "0")
end
end
if next(box.post) then
if ( box.post.apply) then
if general.is_ip_client() then
local saveset = {}
ipv6_enabled:save_check_value( saveset)
ipv6_active_rapid_commit:save_check_value(saveset)
errcode, errmsg = box.set_config( saveset)
if errcode ~= 0 then
g_val.errmsg = errmsg
end
elseif ( val.validate(g_val) == val.ret.ok) then
local saveset = {}
ipv6_enabled:save_check_value( saveset)
if ipv6_enabled:var_exist() then
ipv6_mode:save_value( saveset)
if ((ipv6_mode:get_value() == "ipv6_automatic" or ipv6_mode:get_value() == "ipv6_lte" ) and (ipv6_hidden_ipv4:get_value() ~= "1" )) then
ipv6_mode:save_value( saveset)
elseif ((ipv6_mode:get_value() == "1") and (ipv6_hidden_ipv6_native:get_value() ~= "1")) then
ipv6_mode:save_value( saveset, box.post.native_mode)
save_DS_Lite( saveset)
if (ipv6_mode:get_value() ~= 'ipv6_static') then
ipv6_active_rapid_commit:save_check_value(saveset)
ipv6_active_prefix:save_check_value(saveset)
if (box.post[ipv6_active_prefix:get_var_name()]) then
ipv6_prefixlen:save_value(saveset)
end
end
elseif ( ipv6_mode:get_value() == "2") then
ipv6_mode:save_value( saveset, box.post.tunnel_mode)
end
local tt=ipv6_mode:get_value()
if ((ipv6_mode:get_value() == 'ipv6_static') and (ipv6_hidden_ipv6_native:get_value() ~= "1" )) then
saveIPv6NativeStatics( saveset)
elseif ( ipv6_mode:get_value() == "ipv6_sixxs_heartbeat" ) then
saveIPv6TunnelSixxs( saveset)
elseif ( ipv6_mode:get_value() == "ipv6_6rd" ) then
saveIPv6Tunnel6rd( saveset)
elseif ( ipv6_mode:get_value() == "ipv6_6to4static" ) then
saveIPv6Tunnel6to4( saveset)
end
saveMutUlaPart(saveset)
end
errcode, errmsg = box.set_config( saveset)
if errcode ~= 0 then
g_val.errmsg = errmsg
end
else
if ipv6_enabled:var_exist() then
ipv6_enabled:set_value( "1")
if ((ipv6_mode:get_value() == "ipv6_automatic" or ipv6_mode:get_value() == "ipv6_lte") and (ipv6_hidden_ipv4:get_value() ~= "1")) then
ipv6_mode:set_value( box.post[ipv6_mode:get_var_name()])
elseif ((ipv6_mode:get_value() == "1") and (ipv6_hidden_ipv6_native:get_value() ~= "1")) then
ipv6_mode:set_value( box.post["native_mode"])
rewrite_DS_Lite()
elseif ( ipv6_mode:get_value() == "2") then
ipv6_mode:set_value( box.post["tunnel_mode"])
end
if ((ipv6_mode:get_value() == 'ipv6_static') and (ipv6_hidden_ipv6_native:get_value() ~= "1")) then
rewriteIPv6NativeStatics()
elseif ( ipv6_mode:get_value() == "ipv6_sixxs_heartbeat" ) then
rewriteIPv6TunnelSixxs()
elseif ( ipv6_mode:get_value() == "ipv6_6rd" ) then
rewriteIPv6Tunnel6rd()
elseif ( ipv6_mode:get_value() == "ipv6_6to4static" ) then
rewriteIPv6Tunnel6to4()
end
rewriteMutUlaPart()
else
ipv6_enabled:set_value( "0")
end
end
end
end
?>
<?include "templates/html_head.html" ?>
<style type="text/css">
.formular .formular .formular .formular .labelWidth {
width: 100px;
}
@media (max-width: 700px) and (min-width: 500px) {
.formular.label_input_label_group label:first-child {
width: 100%;
}
#uiView_ipv6_wan_prefix, #uiView_ipv6_wan_ifid {
margin-left: 11.3rem;
}
}
main .formular .formular input[type=radio]+label {
width:auto;
}
</style>
<script type="text/javascript" src="/js/validate.js"></script>
<script type="text/javascript">
<?lua
val.write_js_error_strings()
?>
var g_nConnectMode = 0;
var g_szIPv6Mode = '';
var g_szIPv6Mode_Native = 'ipv6_native';
var g_szIPv6Mode_Tunnel = 'ipv6_tunnel';
var g_isIpClient = <?lua box.out(tostring(general.is_ip_client())) ?>;
function init() {
jxl.display( "uiShow_MoreInfo", "<?lua box.js( ipv6_state:get_value()) ?>" == "5");
OnChange_IPv6Activated();
if (!g_isIpClient) {
OnChange_ConnectionMode( "<?lua box.js( ipv6_mode:get_value()) ?>");
OnChange_MTU( "<?lua box.js( ipv6_use_fixed_mtu:get_value()) ?>" == "1");
}
}
function OnChange_IPv6Activated() {
jxl.display( "uiIPv6Settings", jxl.getChecked(<?lua box.out( [["]]..ipv6_enabled:get_var_name_js()..[["]]) ?>));
}
function onIPv6ReleaseChange(ipv6Release) {
jxl.setValue("<?lua box.out(ipv6_mode:get_var_name_js()..[[_0]]) ?>", ipv6Release);
}
function OnChange_ConnectionMode( szConnectMode) {
switch (szConnectMode) {
default:
case "0":
case "ipv6_automatic":
case "ipv6_lte":
g_nConnectMode = 0;
if ( szConnectMode == "0") {
szConnectMode = jxl.getValue("<?lua box.out(ipv6_mode:get_var_name_js()..[[_0]]) ?>");
}
g_szIPv6Mode = szConnectMode;
jxl.setChecked( "ui_ConnectionMode_Automatic", true);
if (<?lua box.js(config.LTE) ?>) {
jxl.setChecked( "uiRelease8", szConnectMode == "ipv6_lte");
jxl.setChecked( "uiRelease10", szConnectMode == "ipv6_automatic");
jxl.display( "uiIPv6Support", true);
}
jxl.display( "uiShow_DS_Lite", false);
jxl.display( "uiShow_IPv6Settings_Details", false);
jxl.display( "uiShow_IPv6Settings_Tunnel", false);
jxl.display( "uiShow_IPv6Settings_Native", false);
break;
case "1":
case "ipv6_native":
case "ipv6_cable":
case "ipv6_native_unnumbered":
case "ipv6_static":
g_nConnectMode = 1;
jxl.setChecked( "uiView_ConnectionMode_Native", true);
if ( szConnectMode == "1") {
szConnectMode = g_szIPv6Mode_Native;
}
if (<?lua box.out(ipv6_hidden_ipv6_native:get_value() ~= "1" and ipv6_hidden_ds_lite:get_value() ~= "1") ?>)
OnChange_IPv4DSLite(jxl.getChecked("<?lua box.out(tostring(ipv6_ipv4_mode:get_var_name_js())) ?>"));
OnChange_IPv6Mode_Native( szConnectMode);
jxl.display( "uiIPv6Support", false);
jxl.display( "uiShow_DS_Lite", true);
jxl.display( "uiShow_IPv6Settings_Tunnel", false);
jxl.display( "uiShow_IPv6Settings_Native", true);
jxl.display( "uiShow_IPv6Settings_Details", true);
break;
case "2":
case "ipv6_tunnel":
case "ipv6_sixxs_heartbeat":
case "ipv6_6rd":
case "ipv6_6to4static":
g_nConnectMode = 2;
if ( szConnectMode == "2") {
szConnectMode = g_szIPv6Mode_Tunnel;
}
OnChange_IPv6Mode_Tunnel( szConnectMode);
jxl.setChecked( "ui_ConnectionMode_Tunnel", true);
jxl.display( "uiIPv6Support", false);
jxl.display( "uiShow_DS_Lite", false);
jxl.display( "uiShow_IPv6Settings_Native", false);
jxl.display( "uiShow_IPv6Settings_Tunnel", true);
jxl.display( "uiShow_IPv6Settings_Details", true);
break;
}
}
function OnChange_IPv6Mode_Native( szMode) {
g_szIPv6Mode_Native = szMode;
g_szIPv6Mode = szMode;
NativeModeElements( szMode);
switch (szMode) {
default:
case "ipv6_native":
jxl.setChecked( "ui_IPv6ModeNative_Auto", true);
break;
case "ipv6_cable":
jxl.setChecked( "ui_IPv6ModeNative_Cable", true);
break;
case "ipv6_native_unnumbered":
jxl.setChecked( "ui_IPv6ModeNative_Unnumbered", true);
break;
case "ipv6_static":
jxl.setChecked( "ui_IPv6ModeNative_Static", true);
break;
}
}
function OnChange_IPv6Mode_Tunnel( szMode) {
g_szIPv6Mode_Tunnel = szMode;
g_szIPv6Mode = szMode;
TunnelModeElements( szMode);
switch (szMode) {
default:
case "ipv6_tunnel":
jxl.setChecked( "ui_IPv6ModeTunnel_Tunnel", true);
break;
case "ipv6_sixxs_heartbeat":
jxl.setChecked( "ui_IPv6ModeTunnel_SixXS", true);
break;
case "ipv6_6rd":
jxl.setChecked( "ui_IPv6ModeTunnel_6rd", true);
break;
case "ipv6_6to4static":
jxl.setChecked( "ui_IPv6ModeTunnel_6to4static", true);
break;
}
}
function OnChange_DSLiteMode( szValue) {
jxl.setChecked( <?lua box.out( [["]]..ipv6_ipv4_mode:get_var_name_js()..[[_Auto"]]) ?>, (szValue == "ipv4_ds_lite"));
jxl.setChecked( <?lua box.out( [["]]..ipv6_ipv4_mode:get_var_name_js()..[[_Static"]]) ?>, (szValue == "ipv4_4to6static"));
jxl.disableNode( <?lua box.out( [["div_]]..ipv6_ipv4_mode:get_var_name_js()..[[_Static"]]) ?>, ( szValue != "ipv4_4to6static"));
if (szValue == "ipv4_4to6static"){
if (jxl.getChecked( <?lua box.out( [["]]..ipv6_ipv4_mode:get_var_name_js()..[[_Static_IPV6"]]) ?>))
OnChange_AFTRMode("IPV6");
else
OnChange_AFTRMode("FQDN");
}
}
function OnChange_IPv4DSLite( bChecked) {
jxl.setDisabled( <?lua box.out( [["]]..ipv6_ipv4_mode:get_var_name_js()..[[_Auto"]]) ?>, !bChecked);
jxl.setDisabled( <?lua box.out( [["]]..ipv6_ipv4_mode:get_var_name_js()..[[_Static"]]) ?>, !bChecked);
if ( bChecked && ( jxl.getChecked( <?lua box.out( [["]]..ipv6_ipv4_mode:get_var_name_js()..[[_Static"]]) ?>)) ) {
jxl.setDisabled( <?lua box.out( [["]]..ipv6_aftr:get_var_name_js()..[["]]) ?>, false);
if (jxl.getChecked( <?lua box.out( [["]]..ipv6_ipv4_mode:get_var_name_js()..[[_Auto"]]) ?>))
OnChange_DSLiteMode('ipv4_ds_lite');
else
OnChange_DSLiteMode('ipv4_4to6static');
} else {
jxl.setDisabled( <?lua box.out( [["]]..ipv6_aftr:get_var_name_js()..[["]]) ?>, true);
jxl.disableNode( <?lua box.out( [["div_]]..ipv6_ipv4_mode:get_var_name_js()..[[_Static"]]) ?>, true);
}
}
function OnChange_AFTRMode( szValue) {
jxl.setDisabled( <?lua box.out( [["]]..ipv6_aftr:get_var_name_js()..[[_IPV6"]]) ?>, ( szValue != "IPV6"));
jxl.setDisabled( <?lua box.out( [["]]..ipv6_aftr:get_var_name_js()..[[_FQDN"]]) ?>, ( szValue != "FQDN"));
}
function OnChange_prefix( bChecked) {
jxl.setDisabled( <?lua box.out( [["]]..ipv6_prefixlen:get_var_name_js()..[["]]) ?>, !bChecked);
}
function NativeModeElements( szIPv6Mode) {
jxl.setDisabled( <?lua box.out( [["]]..ipv6_static_prefix:get_var_name_js()..[["]]) ?>, szIPv6Mode != 'ipv6_static');
jxl.setDisabled( <?lua box.out( [["]]..ipv6_static_prefixlen:get_var_name_js()..[["]]) ?>, szIPv6Mode != 'ipv6_static');
jxl.setDisabled(<?lua box.out( [["]]..ipv6_wan_use_first_prefix:get_var_name_js()..[[_1"]]) ?>, szIPv6Mode != 'ipv6_static');
jxl.setDisabled(<?lua box.out( [["]]..ipv6_wan_use_first_prefix:get_var_name_js()..[[_0"]]) ?>, szIPv6Mode != 'ipv6_static');
jxl.setDisabled(<?lua box.out( [["]]..ipv6_wan_prefix:get_var_name_js()..[["]]) ?>, szIPv6Mode != 'ipv6_static');
if ( szIPv6Mode == 'ipv6_static') {
if ( jxl.getChecked( <?lua box.out( [["]]..ipv6_wan_use_first_prefix:get_var_name_js()..[[_1"]]) ?>) ) {
OnChange_WanPrefix( '1');
} else {
OnChange_WanPrefix( '0');
}
}
jxl.setDisabled(<?lua box.out( [["]]..ipv6_wan_ifid_automatic:get_var_name_js()..[[_1"]]) ?>, szIPv6Mode != 'ipv6_static');
jxl.setDisabled(<?lua box.out( [["]]..ipv6_wan_ifid_automatic:get_var_name_js()..[[_0"]]) ?>, szIPv6Mode != 'ipv6_static');
jxl.setDisabled(<?lua box.out( [["]]..ipv6_wan_ifid:get_var_name_js()..[["]]) ?>, szIPv6Mode != 'ipv6_static');
if ( szIPv6Mode == 'ipv6_static') {
if ( jxl.getChecked( <?lua box.out( [["]]..ipv6_wan_ifid_automatic:get_var_name_js()..[[_1"]]) ?>)) {
OnChange_WanInterfaceId( '1');
} else {
OnChange_WanInterfaceId( '0');
}
}
jxl.setDisabled(<?lua box.out( [["]]..ipv6_wan_first_dns:get_var_name_js()..[["]]) ?>, szIPv6Mode != 'ipv6_static');
jxl.setDisabled(<?lua box.out( [["]]..ipv6_wan_second_dns:get_var_name_js()..[["]]) ?>, szIPv6Mode != 'ipv6_static');
jxl.display("native_mode_static_elements", szIPv6Mode == 'ipv6_static');
jxl.display("native_mode_nonstatic_elements", szIPv6Mode != 'ipv6_static');
}
function OnChange_WanPrefix( szValue) {
jxl.setDisabled(<?lua box.out( [["]]..ipv6_wan_prefix:get_var_name_js()..[["]]) ?>, szValue != '0');
}
function OnChange_WanInterfaceId( szValue) {
jxl.setDisabled(<?lua box.out( [["]]..ipv6_wan_ifid:get_var_name_js()..[["]]) ?>, szValue != '0');
}
function TunnelModeElements( szIPv6Mode) {
jxl.setDisabled(<?lua box.out( [["]]..ipv6_sixxs_username:get_var_name_js()..[["]]) ?>, szIPv6Mode != 'ipv6_sixxs_heartbeat');
jxl.setDisabled(<?lua box.out( [["]]..ipv6_sixxs_passwd:get_var_name_js()..[["]]) ?>, szIPv6Mode != 'ipv6_sixxs_heartbeat');
jxl.setDisabled(<?lua box.out( [["]]..ipv6_sixxs_tunnelid:get_var_name_js()..[["]]) ?>, szIPv6Mode != 'ipv6_sixxs_heartbeat');
for ( i=0; i<4; i++) {
jxl.setDisabled("ui_6rdEndPoint_"+i, szIPv6Mode != 'ipv6_6rd');
}
jxl.setDisabled(<?lua box.out( [["]]..ipv6_6rd_prefix:get_var_name_js()..[["]]) ?>, szIPv6Mode != 'ipv6_6rd');
jxl.setDisabled(<?lua box.out( [["]]..ipv6_6rd_prefixlen:get_var_name_js()..[["]]) ?>, szIPv6Mode != 'ipv6_6rd');
jxl.setDisabled(<?lua box.out( [["]]..ipv6_6rd_ipv4masklen:get_var_name_js()..[["]]) ?>, szIPv6Mode != 'ipv6_6rd');
for ( i=0; i<4; i++) {
jxl.setDisabled("ui_6to4EndPoint_"+i, szIPv6Mode != 'ipv6_6to4static');
}
jxl.setDisabled(<?lua box.out( [["]]..ipv6_6to4static_remote:get_var_name_js()..[["]]) ?>, szIPv6Mode != 'ipv6_6to4static');
jxl.setDisabled(<?lua box.out( [["]]..ipv6_6to4static_local:get_var_name_js()..[["]]) ?>, szIPv6Mode != 'ipv6_6to4static');
jxl.setDisabled(<?lua box.out( [["]]..ipv6_6to4static_prefix:get_var_name_js()..[["]]) ?>, szIPv6Mode != 'ipv6_6to4static');
jxl.setDisabled(<?lua box.out( [["]]..ipv6_6to4static_prefixlen:get_var_name_js()..[["]]) ?>, szIPv6Mode != 'ipv6_6to4static');
}
function OnChange_MTU( checked) {
jxl.setDisabled(<?lua box.out( [["]]..ipv6_mtu_override:get_var_name_js()..[["]]) ?>, !checked);
}
function On_MainFormSubmit() {
<?lua
val.write_js_checks( g_val)
?>
var maxMTU = 1500;
if (!g_isIpClient && jxl.getChecked("<?lua box.out(ipv6_use_fixed_mtu:get_var_name_js()) ?>") && jxl.getValue("<?lua box.out(ipv6_mtu_override:get_var_name_js()) ?>") > maxMTU ) {
var confirmed = confirm("{?683:487?}");
return confirmed;
}
}
ready.onReady(val.init(On_MainFormSubmit, "apply", "main_form" ));
ready.onReady(init);
</script>
<?include "templates/page_head.html" ?>
<form name="main_form" method="POST" action="/internet/ipv6.lua" id="uiMainForm">
<p>{?683:6?}</p>
<hr>
<p><b>{?683:294?}</b></p>
<?lua
box.out( elem._checkbox( ipv6_enabled:get_var_name(), ipv6_enabled:get_var_name_js(), ipv6_enabled:get_value(), (ipv6_enabled:get_value() == "1"), [[onclick="OnChange_IPv6Activated()"]]))
box.out( [[&nbsp;]])
box.out( elem._label( ipv6_enabled:get_var_name_js(), "Label"..ipv6_enabled:get_var_name_js(), [[{?683:442?}]]))
box.out( [[<div id="uiIPv6Settings" style="">]])
if general.is_ip_client() then
box.out( [[<div class="formular">]])
box.out( elem._checkbox( ipv6_active_rapid_commit:get_var_name(), ipv6_active_rapid_commit:get_var_name_js(), ipv6_active_rapid_commit:get_value(), (ipv6_active_rapid_commit:get_value() == "1")))
box.out( [[&nbsp;]])
box.out( elem._label( ipv6_active_rapid_commit:get_var_name_js(), "Label"..ipv6_active_rapid_commit:get_var_name_js(), [[{?683:604?}]]))
box.out([[</div>]])
box.out([[</div>]])
else
box.out( [[<p><b>{?683:857?}</b></p>]])
box.out( [[<div class="formular">]])
if ( ipv6_hidden_ipv4:get_value() ~= "1" ) then
box.out( elem._radio( ipv6_mode:get_var_name(),ipv6_mode:get_var_name_js()..[[_0]],(ipv6_mode:get_value() == "ipv6_automatic" and "ipv6_automatic") or (config.LTE and "ipv6_lte") or "ipv6_automatic",(ipv6_mode:get_value() == "ipv6_automatic" or ipv6_mode:get_value() == "ipv6_lte"), [[onclick="OnChange_ConnectionMode('0')"]] ))
box.out( [[&nbsp;]])
box.out( elem._label( ipv6_mode:get_var_name_js()..[[_0]], "Label"..ipv6_mode:get_var_name_js()..[[_0]], [[{?683:437?}]]))
box.out( [[<p class="formular">{?683:467?}</p>]])
if ( config.LTE ) then
box.out( [[<div id="uiIPv6Support">]])
box.out( [[<p class="formular">{?683:749?}</p>]])
box.out( [[<div class="formular">]])
box.out( [[<input type="radio" id="uiRelease8" name="releaseVersion" value="ipv6_lte" onclick="onIPv6ReleaseChange('ipv6_lte')" ]] )
if ipv6_mode:get_value() == "ipv6_lte" then
box.out( [[checked="checked"]] )
end
box.out( [[>&nbsp;]])
box.out( [[<label for="uiRelease8">{?683:117?}</label><br>]])
box.out( [[<input type="radio" id="uiRelease10" name="releaseVersion" value="ipv6_automatic" onclick="onIPv6ReleaseChange('ipv6_automatic')"]] )
if ipv6_mode:get_value() == "ipv6_automatic" then
box.out( [[checked="checked"]] )
end
box.out( [[>&nbsp;]])
box.out( [[<label for="uiRelease10">{?683:825?}</label>]])
box.out( [[</div>]])
box.out( [[</div><br>]])
end
end
if ( ipv6_hidden_ipv6_native:get_value() ~= "1" ) then
box.out( elem._radio( ipv6_mode:get_var_name(),ipv6_mode:get_var_name_js()..[[_1]],"1",IsIPv6NativeMode(ipv6_mode:get_value()), [[onclick="OnChange_ConnectionMode('1')"]] ))
box.out( [[&nbsp;]])
box.out( elem._label( ipv6_mode:get_var_name_js()..[[_1]], "Label"..ipv6_mode:get_var_name_js()..[[_1]], [[{?683:258?}]]))
box.out( [[<p class="formular">{?683:623?}</p>]])
if ( ipv6_hidden_ds_lite:get_value() ~= "1" ) then
box.out( [[<div class="formular" id="uiShow_DS_Lite" style="">]])
--box.out( [[<div class="formular">]])
box.out( elem._checkbox( ipv6_ipv4_mode:get_var_name(), ipv6_ipv4_mode:get_var_name_js(), ipv6_ipv4_mode:get_value(), (ipv6_ipv4_mode:get_value() ~= "ipv4_normal"), [[onclick="OnChange_IPv4DSLite(this.checked)"]]))
box.out( [[&nbsp;]])
box.out( elem._label( ipv6_ipv4_mode:get_var_name_js(), "Label"..ipv6_ipv4_mode:get_var_name_js(), [[{?683:198?}]]))
box.out( [[<div class="formular">]])
box.out( elem._radio( ipv6_ipv4_mode:get_var_name()..[[_auto]],ipv6_ipv4_mode:get_var_name_js()..[[_Auto]],"ipv4_ds_lite",(ipv6_ipv4_mode:get_value() ~= "ipv4_4to6static"), [[onclick="OnChange_DSLiteMode('ipv4_ds_lite')"]] ))
box.out( [[&nbsp;]])
box.out( elem._label( ipv6_ipv4_mode:get_var_name_js()..[[_Auto]], "Label"..ipv6_ipv4_mode:get_var_name_js()..[[_Auto]], [[{?683:9169?}]]))
box.out( [[</div>]])
box.out( [[<div class="formular">]])
box.out( elem._radio( ipv6_ipv4_mode:get_var_name()..[[_static]],ipv6_ipv4_mode:get_var_name_js()..[[_Static]],"ipv4_4to6static",(ipv6_ipv4_mode:get_value() == "ipv4_4to6static"), [[onclick="OnChange_DSLiteMode('ipv4_4to6static')"]] ))
box.out( [[&nbsp;]])
box.out( elem._label( ipv6_ipv4_mode:get_var_name_js()..[[_Static]], "Label"..ipv6_ipv4_mode:get_var_name_js()..[[_Static]], [[{?683:379?}]]))
box.out( [[&nbsp;]])
box.out( [[<div class="formular" id="div_]]..ipv6_ipv4_mode:get_var_name_js()..[[_Static">]])
box.out( elem._radio( ipv6_ipv4_mode:get_var_name()..[[_static_aftr]],ipv6_ipv4_mode:get_var_name_js()..[[_Static_IPV6]],"IPV6",is_ipv6_addr(ipv6_aftr:get_value()), [[onclick="OnChange_AFTRMode('IPV6')"]]))
box.out( [[&nbsp;]])
box.out( elem._label( ipv6_ipv4_mode:get_var_name_js()..[[_Static_IPV6]], "Label"..ipv6_ipv4_mode:get_var_name_js()..[[_Static]], [[{?683:677?}]],"","labelWidth"))
box.out( [[&nbsp;]])
box.out( elem._input_plus( "text", ipv6_aftr:get_var_name()..[[_ipv6]], ipv6_aftr:get_var_name_js()..[[_IPV6]], write_value("ipv6"), "24", "50", is_aftr_disabled("ipv6"), val.get_attrs( g_val, ipv6_aftr:get_var_name_js(), ipv6_aftr:get_var_name())))
val.write_html_msg(g_val, ipv6_aftr:get_var_name_js())
box.out( [[<br>]])
box.out( elem._radio( ipv6_ipv4_mode:get_var_name()..[[_static_aftr]],ipv6_ipv4_mode:get_var_name_js()..[[_Static_FQDN]],"FQDN", not is_ipv6_addr(ipv6_aftr:get_value()), [[onclick="OnChange_AFTRMode('FQDN')"]]))
box.out( [[&nbsp;]])
box.out( elem._label( ipv6_ipv4_mode:get_var_name_js()..[[_Static_FQDN]], "Label"..ipv6_ipv4_mode:get_var_name_js()..[[_Static_FQDN]], [[{?683:301?}]],"","labelWidth"))
box.out( [[&nbsp;]])
box.out( elem._input_plus( "text", ipv6_aftr:get_var_name()..[[_fqdn]], ipv6_aftr:get_var_name_js()..[[_FQDN]], write_value("fqdn"), "24", "50", is_aftr_disabled("fqdn")))
box.out( [[</div>]])
box.out( [[</div>]])
--box.out( [[</div>]])
box.out( [[</div>]])
end
end
box.out( elem._radio( ipv6_mode:get_var_name(),ipv6_mode:get_var_name_js()..[[_2]],"2",IsIPv6TunnelMode(ipv6_mode:get_value()), [[onclick="OnChange_ConnectionMode('2')"]] ))
box.out( [[&nbsp;]])
box.out( elem._label( ipv6_mode:get_var_name_js()..[[_2]], "Label"..ipv6_mode:get_var_name_js()..[[_2]], [[{?683:416?}]]))
box.out([[<p class="formular">{?683:545?}</p>]])
box.out([[</div>]])
box.out([[<div id="uiShow_IPv6Settings_Details" style="">]])
box.out([[<p><b>{?683:711?}</b></p>]])
box.out([[<div class="formular" id="uiShow_IPv6Settings_Native" style="">]])
box.out( elem._radio( "native_mode","ui_IPv6ModeNative_Auto","ipv6_native", IsCurrentIPv6Mode( "ipv6_native", ipv6_mode:get_value()), [[onclick="OnChange_IPv6Mode_Native( 'ipv6_native')"]] ))
box.out( [[&nbsp;]])
box.out( elem._label( "ui_IPv6ModeNative_Auto", "Label".."ui_IPv6ModeNative_Auto", [[{?683:7423?}]]))
box.out([[<p class="formular">{?683:823?}</p>]])
box.out( elem._radio( "native_mode","ui_IPv6ModeNative_Cable","ipv6_cable", IsCurrentIPv6Mode( "ipv6_cable", ipv6_mode:get_value()), [[onclick="OnChange_IPv6Mode_Native( 'ipv6_cable')"]] ))
box.out( [[&nbsp;]])
box.out( elem._label( "ui_IPv6ModeNative_Cable", "Label".."ui_IPv6ModeNative_Cable", [[{?683:227?}]]))
box.out( [[<p class="formular">{?683:426?}</p>]])
box.out( elem._radio( "native_mode","ui_IPv6ModeNative_Unnumbered","ipv6_native_unnumbered", IsCurrentIPv6Mode( "ipv6_native_unnumbered", ipv6_mode:get_value()), [[onclick="OnChange_IPv6Mode_Native( 'ipv6_native_unnumbered')"]] ))
box.out( [[&nbsp;]])
box.out( elem._label( "ui_IPv6ModeNative_Unnumbered", "Label".."ui_IPv6ModeNative_Unnumbered", [[{?683:985?}]]))
box.out( [[<p class="formular">{?683:330?}</p>]])
box.out( elem._radio( "native_mode","ui_IPv6ModeNative_Static","ipv6_static", IsCurrentIPv6Mode( "ipv6_static", ipv6_mode:get_value()), [[onclick="OnChange_IPv6Mode_Native( 'ipv6_static')"]] ))
box.out( [[&nbsp;]])
box.out( elem._label( "ui_IPv6ModeNative_Static", "Label".."ui_IPv6ModeNative_Static", [[{?683:412?}]]))
box.out( [[<div id="native_mode_nonstatic_elements">]])
box.out( [[<hr>]])
box.out( elem._checkbox( ipv6_active_rapid_commit:get_var_name(), ipv6_active_rapid_commit:get_var_name_js(), ipv6_active_rapid_commit:get_value(), (ipv6_active_rapid_commit:get_value() == "1")))
box.out( [[&nbsp;]])
box.out( elem._label( ipv6_active_rapid_commit:get_var_name_js(), "Label"..ipv6_active_rapid_commit:get_var_name_js(), [[{?683:30?}]]))
box.out( [[<br>]])
box.out( elem._checkbox( ipv6_active_prefix:get_var_name(), ipv6_active_prefix:get_var_name_js(), ipv6_active_prefix:get_value(), (ipv6_active_prefix:get_value() == "1"), [[onclick="OnChange_prefix(this.checked)"]]))
box.out( [[&nbsp;]])
box.out( elem._label( ipv6_active_prefix:get_var_name_js(), "Label"..ipv6_active_prefix:get_var_name_js(), [[{?683:818?}]]))
box.out( [[<p class="formular group">]])
box.out( [[{?683:961?}]])
box.out( [[&nbsp;]])
box.out( elem._input_plus( "text", ipv6_prefixlen:get_var_name(), ipv6_prefixlen:get_var_name_js(), ipv6_prefixlen:get_value(), "4", "20", is_disabled(ipv6_active_prefix:get_value() == "0")))
box.out( [[&nbsp;]])
box.out( [[{?683:8?}]])
box.out( [[</p>]])
box.out( [[</div>]])
box.out( [[<div id="native_mode_static_elements">]])
box.out( [[<p class="formular">{?683:179?}</p>]])
box.out( [[<p class="formular group"> <!-- ipv6_static - Prefix/Prefix-length -->]])
box.out( elem._label( ipv6_static_prefix:get_var_name_js(), "Label"..ipv6_static_prefix:get_var_name_js(), [[{?683:141?}]], nil, [[grouplabel]]))
box.out( elem._input( "text", ipv6_static_prefix:get_var_name(), ipv6_static_prefix:get_var_name_js(), ipv6_static_prefix:get_value(), "24", "21", val.get_attrs( g_val, ipv6_static_prefix:get_var_name_js(), ipv6_static_prefix:get_var_name())))
box.out( [[&nbsp;/&nbsp;]])
box.out( elem._input( "text", ipv6_static_prefixlen:get_var_name(), ipv6_static_prefixlen:get_var_name_js(), ipv6_static_prefixlen:get_value(), "4", "2", val.get_attrs( g_val, ipv6_static_prefixlen:get_var_name_js(), ipv6_static_prefixlen:get_var_name())))
val.write_html_msg(g_val, ipv6_static_prefix:get_var_name_js())
val.write_html_msg(g_val, ipv6_static_prefixlen:get_var_name_js())
box.out( [[</p>]])
box.out( [[<p class="formular group"> <!-- ipv6 static Wan-Prefix auto -->]])
box.out( elem._label( ipv6_wan_use_first_prefix:get_var_name_js()..[[_1]], "Label"..ipv6_wan_use_first_prefix:get_var_name_js()..[[_1]], [[{?683:3475?}]], nil, [[grouplabel]]))
box.out( elem._radio( ipv6_wan_use_first_prefix:get_var_name(), ipv6_wan_use_first_prefix:get_var_name_js()..[[_1]], "1", (ipv6_wan_use_first_prefix:get_value() == '1'),[[onclick="OnChange_WanPrefix('1')"]] ))
box.out( [[&nbsp;]])
box.out( elem._label( ipv6_wan_use_first_prefix:get_var_name_js()..[[_1]], [[Label]]..ipv6_wan_use_first_prefix:get_var_name_js()..[[_2]], [[{?683:510?}]], nil, [[grouplabel]]))
box.out( [[</p>]])
box.out( [[<p class="formular group"> <!-- ipv6 static Wan-Prefix manuell -->]])
box.out( elem._label( ipv6_wan_use_first_prefix:get_var_name_js()..[[_0]], "Label"..ipv6_wan_use_first_prefix:get_var_name_js()..[[_3]], [[]], nil, [[grouplabel]]))
box.out( elem._radio( ipv6_wan_use_first_prefix:get_var_name(), ipv6_wan_use_first_prefix:get_var_name_js()..[[_0]], "0", (ipv6_wan_use_first_prefix:get_value() == '0'),[[onclick="OnChange_WanPrefix('0')"]] ))
box.out( [[&nbsp;]])
box.out( elem._label( ipv6_wan_use_first_prefix:get_var_name_js()..[[_0]], [[Label]]..ipv6_wan_use_first_prefix:get_var_name_js()..[[_4]], [[{?683:105?}]], nil, [[grouplabel]]))
box.out( elem._input( "text", ipv6_wan_prefix:get_var_name(), ipv6_wan_prefix:get_var_name_js(), ipv6_wan_prefix:get_value(), "24", "50", val.get_attrs( g_val, ipv6_wan_prefix:get_var_name_js(), ipv6_wan_prefix:get_var_name())))
box.out( [[ / 64]])
val.write_html_msg(g_val, ipv6_wan_prefix:get_var_name_js())
box.out( [[</p>]])
box.out( [[<p class="formular group"> <!-- ipv6 static Wan-interface-ID auto -->]])
box.out( elem._label( ipv6_wan_ifid_automatic:get_var_name_js()..[[_1]], "Label"..ipv6_wan_ifid_automatic:get_var_name_js()..[[_1]], [[{?683:942?}]], nil, [[grouplabel]]))
box.out( elem._radio( ipv6_wan_ifid_automatic:get_var_name(), ipv6_wan_ifid_automatic:get_var_name_js()..[[_1]], "1", (ipv6_wan_ifid_automatic:get_value() == '1'),[[onclick="OnChange_WanInterfaceId('1')"]] ))
box.out( [[&nbsp;]])
box.out( elem._label( ipv6_wan_ifid_automatic:get_var_name_js()..[[_1]], [[Label]]..ipv6_wan_ifid_automatic:get_var_name_js()..[[_2]], [[{?683:689?}]], nil, [[grouplabel]]))
box.out( [[</p>]])
box.out( [[<p class="formular group"> <!-- ipv6 static Wan-interface-ID Manuell -->]])
box.out( elem._label( ipv6_wan_ifid_automatic:get_var_name_js()..[[_0]], "Label"..ipv6_wan_ifid_automatic:get_var_name_js()..[[_3]], [[]], nil, [[grouplabel]]))
box.out( elem._radio( ipv6_wan_ifid_automatic:get_var_name(), ipv6_wan_ifid_automatic:get_var_name_js()..[[_0]], "0", (ipv6_wan_ifid_automatic:get_value() == '0'),[[onclick="OnChange_WanInterfaceId('0')"]] ))
box.out( [[&nbsp;]])
box.out( elem._label( ipv6_wan_ifid_automatic:get_var_name_js()..[[_0]], [[Label]]..ipv6_wan_ifid_automatic:get_var_name_js()..[[_4]], [[{?683:1101?}]], nil, [[grouplabel]]))
box.out( elem._input( "text", ipv6_wan_ifid:get_var_name(), ipv6_wan_ifid:get_var_name_js(), ipv6_wan_ifid:get_value(), "24", "50", val.get_attrs( g_val, ipv6_wan_ifid:get_var_name_js(), ipv6_wan_ifid:get_var_name())))
val.write_html_msg(g_val, ipv6_wan_ifid:get_var_name_js())
box.out( [[</p>]])
box.out( [[<p class="formular"><!--ipv6 static Dns -Server 1-->]])
box.out( elem._label( ipv6_wan_first_dns:get_var_name_js(), "Label"..ipv6_wan_first_dns:get_var_name_js(), [[{?683:826?}]]))
box.out( elem._input( "text", ipv6_wan_first_dns:get_var_name(), ipv6_wan_first_dns:get_var_name_js(), ipv6_wan_first_dns:get_value(), "39", "39", val.get_attrs( g_val, ipv6_wan_first_dns:get_var_name_js(), ipv6_wan_first_dns:get_var_name())))
val.write_html_msg(g_val, ipv6_wan_first_dns:get_var_name_js())
box.out( [[</p>]])
box.out( [[<p class="formular"><!--ipv6 static Dns -Server 2-->]])
box.out( elem._label( ipv6_wan_second_dns:get_var_name_js(), "Label"..ipv6_wan_second_dns:get_var_name_js(), [[{?683:455?}]]))
box.out( elem._input( "text", ipv6_wan_second_dns:get_var_name(), ipv6_wan_second_dns:get_var_name_js(), ipv6_wan_second_dns:get_value(), "39", "39", val.get_attrs( g_val, ipv6_wan_first_dns:get_var_name_js(), ipv6_wan_first_dns:get_var_name())))
val.write_html_msg(g_val, ipv6_wan_second_dns:get_var_name_js())
box.out( [[</p>]])
box.out( [[</div>]])
box.out( [[</div>]])
box.out( [[<div id="uiShow_IPv6Settings_Tunnel" style="">]])
box.out( [[<p>{?683:543?}</p>]])
box.out( [[<div class="formular">]])
box.out( [[<div class="wide">]])
box.out( elem._radio( "tunnel_mode","ui_IPv6ModeTunnel_Tunnel","ipv6_tunnel", IsCurrentIPv6Mode( "ipv6_tunnel", ipv6_mode:get_value()), [[onclick="OnChange_IPv6Mode_Tunnel( 'ipv6_tunnel')"]] ))
box.out( [[&nbsp;]])
box.out( elem._label( "ui_IPv6ModeTunnel_Tunnel", "Label".."ui_IPv6ModeTunnel_Tunnel", [[{?683:35?}]]))
box.out( [[<p class="formular">{?683:892?}</p>]])
box.out( elem._radio( "tunnel_mode","ui_IPv6ModeTunnel_SixXS","ipv6_sixxs_heartbeat", IsCurrentIPv6Mode( "ipv6_sixxs_heartbeat", ipv6_mode:get_value()), [[onclick="OnChange_IPv6Mode_Tunnel( 'ipv6_sixxs_heartbeat')"]] ))
box.out( [[&nbsp;]])
box.out( elem._label( "ui_IPv6ModeTunnel_SixXS", "Label".."ui_IPv6ModeTunnel_SixXS", [[{?683:640?}]]))
box.out( [[<p class="formular">{?683:293?}</p>]])
box.out( [[<div class="formular">]])
box.out( [[<p class="formular">]])
box.out( elem._label( ipv6_sixxs_username:get_var_name_js(), "Label"..ipv6_sixxs_username:get_var_name_js(), [[{?txtUsername?}:]]))
box.out( elem._input( "text", ipv6_sixxs_username:get_var_name(), ipv6_sixxs_username:get_var_name_js(), ipv6_sixxs_username:get_value(), "24", "50", val.get_attrs( g_val, ipv6_sixxs_username:get_var_name_js(), ipv6_sixxs_username:get_var_name())))
val.write_html_msg(g_val, ipv6_sixxs_username:get_var_name_js())
box.out( [[</p>]])
box.out( [[<p class="formular">]])
box.out( elem._label( ipv6_sixxs_passwd:get_var_name_js(), "Label"..ipv6_sixxs_passwd:get_var_name_js(), [[{?txtKennwort?}:]]))
local attribs=val.get_attrs( g_val, ipv6_sixxs_passwd:get_var_name_js(), ipv6_sixxs_passwd:get_var_name())
if attribs==nil then
attribs=""
end
attribs=attribs..[[autocomplete="off"]]
box.out( elem._input( "text", ipv6_sixxs_passwd:get_var_name(), ipv6_sixxs_passwd:get_var_name_js(), ipv6_sixxs_passwd:get_value(), "24", "50", attribs))
val.write_html_msg(g_val, ipv6_sixxs_passwd:get_var_name_js())
box.out( [[</p>]])
box.out( [[<p class="formular">]])
box.out( elem._label( ipv6_sixxs_tunnelid:get_var_name_js(), "Label"..ipv6_sixxs_tunnelid:get_var_name_js(), [[{?683:867?}:]]))
box.out( elem._input( "text", ipv6_sixxs_tunnelid:get_var_name(), ipv6_sixxs_tunnelid:get_var_name_js(), ipv6_sixxs_tunnelid:get_value(), "24", "50", val.get_attrs( g_val, ipv6_sixxs_tunnelid:get_var_name_js(), ipv6_sixxs_tunnelid:get_var_name())))
val.write_html_msg(g_val, ipv6_sixxs_tunnelid:get_var_name_js())
box.out( [[</p>]])
box.out( [[</div>]])
box.out( elem._radio( "tunnel_mode","ui_IPv6ModeTunnel_6rd","ipv6_6rd", IsCurrentIPv6Mode( "ipv6_6rd", ipv6_mode:get_value()), [[onclick="OnChange_IPv6Mode_Tunnel( 'ipv6_6rd')"]] ))
box.out( [[&nbsp;]])
box.out( elem._label( "ui_IPv6ModeTunnel_6rd", "Label".."ui_IPv6ModeTunnel_6rd", [[{?683:56?}]]))
box.out( [[<p class="formular">{?683:191?}</p>]])
box.out( [[<div class="formular">]])
box.out( [[<div class="formular group">]])
box.out( elem._label( "ui_6rdEndPoint_0", "Label".."ui_6rdEndPoint_0", [[{?683:578?}:]], nil, [[grouplabel]]))
g_t_6rd_ipv4 = split( ipv6_6rd_popaddr:get_value(), ".")
box.out( elem._input( "text", "ipv4_6rd_0", "ui_6rdEndPoint_0", g_t_6rd_ipv4[1], "3", "3", val.write_attrs(g_val, 'ui_6rdEndPoint_0')))
box.out( [[&nbsp;:&nbsp;]])
box.out( elem._input( "text", "ipv4_6rd_1", "ui_6rdEndPoint_1", g_t_6rd_ipv4[2], "3", "3", val.write_attrs(g_val, 'ui_6rdEndPoint_1')))
box.out( [[&nbsp;:&nbsp;]])
box.out( elem._input( "text", "ipv4_6rd_2", "ui_6rdEndPoint_2", g_t_6rd_ipv4[3], "3", "3", val.write_attrs(g_val, 'ui_6rdEndPoint_2')))
box.out( [[&nbsp;:&nbsp;]])
box.out( elem._input( "text", "ipv4_6rd_3", "ui_6rdEndPoint_3", g_t_6rd_ipv4[4], "3", "3", val.write_attrs(g_val, 'ui_6rdEndPoint_3')))
val.write_html_msg(g_val, "ui_6rdEndPoint_0", "ui_6rdEndPoint_1", "ui_6rdEndPoint_2", "ui_6rdEndPoint_3")
box.out( [[</div>]])
box.out( [[<div class="formular group">]])
box.out( elem._label( ipv6_6rd_prefix:get_var_name_js(), "Label"..ipv6_6rd_prefix:get_var_name_js(), [[{?683:123?}]], nil, [[grouplabel]]))
box.out( elem._input( "text", ipv6_6rd_prefix:get_var_name(), ipv6_6rd_prefix:get_var_name_js(), ipv6_6rd_prefix:get_value(), "24", "21", val.get_attrs( g_val, ipv6_6rd_prefix:get_var_name_js(), ipv6_6rd_prefix:get_var_name())))
box.out( [[&nbsp;/&nbsp;]])
box.out( elem._input( "text", ipv6_6rd_prefixlen:get_var_name(), ipv6_6rd_prefixlen:get_var_name_js(), ipv6_6rd_prefixlen:get_value(), "4", "2", val.get_attrs( g_val, ipv6_6rd_prefixlen:get_var_name_js(), ipv6_6rd_prefixlen:get_var_name())))
val.write_html_msg(g_val, ipv6_6rd_prefixlen:get_var_name_js())
box.out( [[</div>]])
box.out( [[<div class="formular">]])
box.out( elem._label( ipv6_6rd_ipv4masklen:get_var_name_js(), "Label"..ipv6_6rd_ipv4masklen:get_var_name_js(), [[{?683:966?}]]))
box.out( elem._input( "text", ipv6_6rd_ipv4masklen:get_var_name(), ipv6_6rd_ipv4masklen:get_var_name_js(), ipv6_6rd_ipv4masklen:get_value(), "3", "2", val.get_attrs( g_val, ipv6_6rd_ipv4masklen:get_var_name_js(), ipv6_6rd_ipv4masklen:get_var_name())))
val.write_html_msg(g_val, ipv6_6rd_ipv4masklen:get_var_name_js())
box.out( [[</div>]])
box.out( [[</div>]])
box.out( elem._radio( "tunnel_mode","ui_IPv6ModeTunnel_6to4static","ipv6_6to4static", IsCurrentIPv6Mode( "ipv6_6to4static", ipv6_mode:get_value()), [[onclick="OnChange_IPv6Mode_Tunnel( 'ipv6_6to4static')"]] ))
box.out( [[&nbsp;]])
box.out( elem._label( "ui_IPv6ModeTunnel_6to4static", "Label".."ui_IPv6ModeTunnel_6to4static", [[{?683:754?}]]))
box.out( [[<p class="formular">{?683:7392?}</p>]])
box.out( [[<div class="formular">]])
box.out( [[<p class="formular group"><!--6to4static Tunnel Endpunkt-->]])
box.out( elem._label( "ui_6to4EndPoint_0", "Label".."ui_6to4EndPoint_0", [[{?683:739?}:]], nil, [[grouplabel]]))
g_t_6to4_ipv4 = split( ipv6_6to4static_popaddr:get_value(), ".")
box.out( elem._input( "text", "ipv4_6to4_0", "ui_6to4EndPoint_0", g_t_6to4_ipv4[1], "3", "3", val.write_attrs(g_val, 'ui_6to4EndPoint_0')))
box.out( [[&nbsp;:&nbsp;]])
box.out( elem._input( "text", "ipv4_6to4_1", "ui_6to4EndPoint_1", g_t_6to4_ipv4[2], "3", "3", val.write_attrs(g_val, 'ui_6to4EndPoint_1')))
box.out( [[&nbsp;:&nbsp;]])
box.out( elem._input( "text", "ipv4_6to4_2", "ui_6to4EndPoint_2", g_t_6to4_ipv4[3], "3", "3", val.write_attrs(g_val, 'ui_6to4EndPoint_2')))
box.out( [[&nbsp;:&nbsp;]])
box.out( elem._input( "text", "ipv4_6to4_3", "ui_6to4EndPoint_3", g_t_6to4_ipv4[4], "3", "3", val.write_attrs(g_val, 'ui_6to4EndPoint_3')))
val.write_html_msg(g_val, "ui_6to4EndPoint_0", "ui_6to4EndPoint_1", "ui_6to4EndPoint_2", "ui_6to4EndPoint_3")
box.out( [[</p>]])
box.out( [[<p class="formular"><!--6to4static Tunnel Endpunkt-->]])
box.out( elem._label( ipv6_6to4static_remote:get_var_name_js(), "Label"..ipv6_6to4static_remote:get_var_name_js(), [[{?683:385?}]]))
box.out( elem._input( "text", ipv6_6to4static_remote:get_var_name(), ipv6_6to4static_remote:get_var_name_js(), ipv6_6to4static_remote:get_value(), "39", "50", val.get_attrs( g_val, ipv6_6to4static_remote:get_var_name_js(), ipv6_6to4static_remote:get_var_name())))
val.write_html_msg(g_val, ipv6_6to4static_remote:get_var_name_js())
box.out( [[</p>]])
box.out( [[<p class="formular"><!--6to4static Tunnel Endpunkt-->]])
box.out( elem._label( ipv6_6to4static_local:get_var_name_js(), "Label"..ipv6_6to4static_local:get_var_name_js(), [[{?683:8918?}]]))
box.out( elem._input( "text", ipv6_6to4static_local:get_var_name(), ipv6_6to4static_local:get_var_name_js(), ipv6_6to4static_local:get_value(), "39", "50", val.get_attrs( g_val, ipv6_6to4static_local:get_var_name_js(), ipv6_6to4static_local:get_var_name())))
val.write_html_msg(g_val, ipv6_6to4static_local:get_var_name_js())
box.out( [[</p>]])
box.out( [[<p class="formular group"><!--6to4static Tunnel Endpunkt-->]])
box.out( elem._label( ipv6_6to4static_prefix:get_var_name_js(), "Label"..ipv6_6to4static_prefix:get_var_name_js(), [[{?683:526?}]], nil, [[grouplabel]]))
box.out( elem._input( "text", ipv6_6to4static_prefix:get_var_name(), ipv6_6to4static_prefix:get_var_name_js(), ipv6_6to4static_prefix:get_value(), "24", "21", val.get_attrs( g_val, ipv6_6to4static_prefix:get_var_name_js(), ipv6_6to4static_prefix:get_var_name())))
box.out( [[&nbsp;/&nbsp;]])
box.out( elem._input( "text", ipv6_6to4static_prefixlen:get_var_name(), ipv6_6to4static_prefixlen:get_var_name_js(), ipv6_6to4static_prefixlen:get_value(), "4", "2", val.get_attrs( g_val, ipv6_6to4static_prefixlen:get_var_name_js(), ipv6_6to4static_prefixlen:get_var_name())))
val.write_html_msg(g_val, ipv6_6to4static_prefixlen:get_var_name_js())
box.out( [[</p>]])
box.out( [[</div>]])
box.out( [[</div>]])
box.out( [[</div>]])
box.out( [[</div>]])
box.out( [[</div>]])
box.out( [[<hr>]])
box.out( [[<p class="mb10"><b>{?683:854?}</b></p>]])
box.out( [[<p class="group">]])
box.out( elem._checkbox( ipv6_use_fixed_mtu:get_var_name(), ipv6_use_fixed_mtu:get_var_name_js(), ipv6_use_fixed_mtu:get_value(), (ipv6_use_fixed_mtu:get_value() == "1"),[[onclick="OnChange_MTU(this.checked)"]]))
box.out( [[&nbsp;]])
box.out( elem._label( ipv6_use_fixed_mtu:get_var_name_js(), "Label"..ipv6_use_fixed_mtu:get_var_name_js(), [[{?683:7036?}]], nil, [[grouplabel]]))
box.out( [[&nbsp;]])
box.out( elem._input( "text", ipv6_mtu_override:get_var_name(), ipv6_mtu_override:get_var_name_js(), ipv6_mtu_override:get_value(), "8", "5", val.get_attrs( g_val, ipv6_mtu_override:get_var_name_js(), ipv6_mtu_override:get_var_name())))
box.out( [[&nbsp;]])
box.out( elem._label( ipv6_mtu_override:get_var_name_js(), "Label"..ipv6_mtu_override:get_var_name_js(), [[{?683:900?}]]))
box.out( [[</p>]])
end
?>
</div>
<div id="btn_form_foot">
<input type="hidden" name="sid" value="<?lua box.html(box.glob.sid) ?>">
<button type="submit" name="apply" id="uiApply">{?txtApply?}</button>
<button type="submit" name="cancel">{?txtCancel?}</button>
</div>
</form>
<?include "templates/page_end.html" ?>
<?include "templates/html_end.html" ?>
