#ifndef __VLAN_HOOK_DEF_H__
#define __VLAN_HOOK_DEF_H__

extern s32_t my_custom_vlan_set_hook(struct netif* netif, struct pbuf* pbuf, const struct eth_addr* src, const struct eth_addr* dst, u16_t eth_type);
extern s32_t my_custom_vlan_check_hook(struct netif* netif, struct eth_hdr* ethhdr, struct eth_vlan_hdr* vlan_hdr);

#endif /* !__VLAN_HOOK_DEF_H__ */
