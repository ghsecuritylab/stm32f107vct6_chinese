#define ETH_MAX_NETIFS_COUNT    10
/* ... */
static struct netif *assigned_netifs[ETH_MAX_NETIFS_COUNT];
static int assigned_netifs_count;

/* ... */
err_t ethernetif_init(struct netif *netif)
{
  if (assigned_netifs_count >= ETH_MAX_NETIFS_COUNT)
    return ERR_MEM;

  // ...
  assigned_netifs[assigned_netifs_count] = netif;
  assigned_netifs_count++;
  // ...
}

/* ... */
static void
smart_pass_incoming_packet_to_lwip(struct pbuf *input_pbuf)
{
  if (input_pbuf->len > SIZEOF_ETH_HDR)
  {
    switch (((struct eth_hdr *)input_pbuf->payload)->type)
    {
    case PP_HTONS(ETHTYPE_IP):
      {
        /* smart search of matching netif */
        struct ip_hdr *iphdr;
        u16_t iphdr_hlen;
        ipX_addr_t iphdr_dest;
        SYS_ARCH_DECL_PROTECT(old_level);
        struct netif *xnetif;
        char netif_found;

        if (input_pbuf->len < (SIZEOF_ETH_HDR + IP_HLEN))
          goto ip_netif_search_end;
        iphdr = (struct ip_hdr *)((char *)input_pbuf->payload + SIZEOF_ETH_HDR);
        if (IPH_V(iphdr) != 4)
          goto ip_netif_search_end;
        iphdr_hlen = IPH_HL(iphdr)*4;
        if (iphdr_hlen > (input_pbuf->len - SIZEOF_ETH_HDR))
          goto ip_netif_search_end;

        ip_addr_copy(*ipX_2_ip(&iphdr_dest), iphdr->dest);
        netif_found = 0;
        SYS_ARCH_PROTECT(old_level);
        for (int i = 0; i < assigned_netifs_count; i++)
        {
          xnetif = assigned_netifs[i];
#if LWIP_IGMP
          if (ip_addr_ismulticast(&iphdr_dest))
          {
            if (igmp_lookfor_group(xnetif, &iphdr_dest))
            {
              netif_found = 1;
            }
          }
          else
#endif /* LWIP_IGMP */
          {
            /* interface is configured? */
            if (!ip_addr_isany(&(xnetif->ip_addr)))
            {
              /* unicast to this interface address? */
              if (ip_addr_cmp(&iphdr_dest, &(xnetif->ip_addr)) ||
              /* or broadcast on this interface network address? */
                  ip_addr_isbroadcast(&iphdr_dest, xnetif))
              {
                netif_found = 1;
              }
#if LWIP_AUTOIP
              /* connections to link-local addresses must persist after changing the netif's address (RFC3927 ch. 1.9) */
              if ((xnetif->autoip != NULL) &&
                  ip_addr_cmp(&iphdr_dest, &(xnetif->autoip->llipaddr)))
              {
                netif_found = 1;
              }
#endif /* LWIP_AUTOIP */
            }
            else
            {
              // only one of assined interfaces may not be configured (dhcp ?)
              netif_found = 1;
            }
          }
          if (netif_found)
            break;
        }
        SYS_ARCH_UNPROTECT(old_level);

        if (netif_found)
        {
          if (xnetif->input(input_pbuf, xnetif) != ERR_OK)
          {
            LWIP_DEBUGF(NETIF_DEBUG | LWIP_DBG_LEVEL_SERIOUS, (DEBUG_HDR"IP packet input error\n"));
            pbuf_free(input_pbuf);
          }
        }
        break;

ip_netif_search_end:
        pbuf_free(input_pbuf);
        break;
      }

    case PP_HTONS(ETHTYPE_ARP):
      {
        /* to all assigned netifs */
        for (int i = 0; i < assigned_netifs_count; i++)
        {
          pbuf_ref(input_pbuf);
          if (assigned_netifs[i]->input(input_pbuf, assigned_netifs[i]) != ERR_OK)
          {
            LWIP_DEBUGF(NETIF_DEBUG | LWIP_DBG_LEVEL_SERIOUS, (DEBUG_HDR"ARP packet input error on assigned netif #%u\n", i));
            pbuf_free(input_pbuf);
            break;
          }
        }
        pbuf_free(input_pbuf);
        break;
      }
    
    default:
      pbuf_free(input_pbuf);
      break;
    }
  }
  else
  {
    pbuf_free(input_pbuf);
  }
}
