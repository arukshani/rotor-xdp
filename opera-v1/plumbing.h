static void load_xdp_program(void)
{
	int nic_ifindex = if_nametoindex(nic_iface);

    struct config cfgs[26];

	int y;
    for (y = 0; y < n_nic_ports; y++)
	{
        // Physical NIC
        struct config nic_cfg = {
            .ifindex = nic_ifindex,
            .ifname = nic_iface,
            .xsk_if_queue = y,
            .xsk_poll_mode = true,
            .filename = "nic_kern.o",
            .progsec = "xdp_sock_1"};

        cfgs[y] = nic_cfg;
    }

    int x, z;
    z = 0;
    int start_index_for_veth_ports = n_nic_ports;
    for (x = start_index_for_veth_ports; x < n_ports; x++)
	{
        int veth_ifindex = if_nametoindex(out_veth_arr[z]);
        struct config veth_cfg = {
		.ifindex = veth_ifindex,
		.ifname = out_veth_arr[z],
		.xsk_if_queue = 0,
		.xsk_poll_mode = true,
		.filename = "veth_kern.o",
		.progsec = "xdp_sock_0"};

        z = z + 1;
        cfgs[x] = veth_cfg;
    }

	//=======================Following is just for NIC========================
    //bcoz we only need one program to load on NIC even though it has multiple queues
	char errmsg[STRERR_BUFSIZE];
	int err;

	printf("xdp_prog[%d] is %s \n", 0, cfgs[0].filename);

	xdp_prog[0] = xdp_program__open_file(cfgs[0].filename, cfgs[0].progsec, NULL);
	err = libxdp_get_error(xdp_prog[0]);
	if (err)
	{
		libxdp_strerror(err, errmsg, sizeof(errmsg));
		fprintf(stderr, "ERROR: program loading failed: %s\n", errmsg);
		exit(EXIT_FAILURE);
	}

	err = xdp_program__attach(xdp_prog[0], cfgs[0].ifindex, XDP_FLAGS_DRV_MODE, 0);
	// err = xdp_program__attach(xdp_prog[0], cfgs[0].ifindex, XDP_FLAGS_SKB_MODE, 0);
	if (err)
	{
		libxdp_strerror(err, errmsg, sizeof(errmsg));
		fprintf(stderr, "ERROR: attaching program failed: %s\n", errmsg);
		exit(EXIT_FAILURE);
	}
	
    //=======================Following is for VETHS========================

	int start_index_for_veth_cfgs = n_nic_ports; 
    int i;
	for (i = start_index_for_veth_cfgs; i < n_ports; i++)
	{

		char errmsg[STRERR_BUFSIZE];
		int err;

		printf("xdp_prog[%d] is %s \n", i, cfgs[i].filename);

		xdp_prog[i] = xdp_program__open_file(cfgs[i].filename, cfgs[i].progsec, NULL);
		err = libxdp_get_error(xdp_prog[i]);
		if (err)
		{
			libxdp_strerror(err, errmsg, sizeof(errmsg));
			fprintf(stderr, "ERROR: program loading failed: %s\n", errmsg);
			exit(EXIT_FAILURE);
		}

		err = xdp_program__attach(xdp_prog[i], cfgs[i].ifindex, XDP_FLAGS_DRV_MODE, 0);
		if (err)
		{
			libxdp_strerror(err, errmsg, sizeof(errmsg));
			fprintf(stderr, "ERROR: attaching program failed: %s\n", errmsg);
			exit(EXIT_FAILURE);
		}
	}
}

static void remove_xdp_program_nic(void)
{
	struct xdp_multiprog *mp;
	int i, err;

	mp = xdp_multiprog__get_from_ifindex(if_nametoindex(nic_iface));
	printf("remove_xdp_program_nic: %s \n", nic_iface);
	if (IS_ERR_OR_NULL(mp))
	{
		printf("No XDP program loaded on %s\n", nic_iface);
		// continue;
	}

	err = xdp_multiprog__detach(mp);
	if (err)
		printf("Unable to detach XDP program: %s\n", strerror(-err));

	// for (i = 0; i < 1; i++)
	// {
	// 	mp = xdp_multiprog__get_from_ifindex(if_nametoindex(port_params[i].iface));
	// 	printf("remove_xdp_program_nic: %s \n", port_params[i].iface);
	// 	if (IS_ERR_OR_NULL(mp))
	// 	{
	// 		printf("No XDP program loaded on %s\n", port_params[i].iface);
	// 		continue;
	// 	}

	// 	err = xdp_multiprog__detach(mp);
	// 	if (err)
	// 		printf("Unable to detach XDP program: %s\n", strerror(-err));
	// }
}

static void remove_xdp_program_veth(void)
{
	struct xdp_multiprog *mp;
	int i, err;

	for (i = n_nic_ports; i < n_ports; i++)
	{
		mp = xdp_multiprog__get_from_ifindex(if_nametoindex(port_params[i].iface));
		printf("remove_xdp_program_veth: %s \n", port_params[i].iface);
		if (IS_ERR_OR_NULL(mp))
		{
			printf("No XDP program loaded on %s\n", port_params[i].iface);
			continue;
		}

		err = xdp_multiprog__detach(mp);
		if (err)
			printf("Unable to detach XDP program: %s\n", strerror(-err));
	}
}


// static enum xdp_attach_mode opt_attach_mode = XDP_MODE_NATIVE;

static int lookup_bpf_map(int prog_fd)
{
	__u32 i, *map_ids, num_maps, prog_len = sizeof(struct bpf_prog_info);
	__u32 map_len = sizeof(struct bpf_map_info);
	struct bpf_prog_info prog_info = {};
	int fd, err, xsks_map_fd = -ENOENT;
	struct bpf_map_info map_info;

	err = bpf_obj_get_info_by_fd(prog_fd, &prog_info, &prog_len);
	if (err)
		return err;

	num_maps = prog_info.nr_map_ids;

	map_ids = calloc(prog_info.nr_map_ids, sizeof(*map_ids));
	if (!map_ids)
		return -ENOMEM;

	memset(&prog_info, 0, prog_len);
	prog_info.nr_map_ids = num_maps;
	prog_info.map_ids = (__u64)(unsigned long)map_ids;

	err = bpf_obj_get_info_by_fd(prog_fd, &prog_info, &prog_len);
	if (err)
	{
		free(map_ids);
		return err;
	}

	for (i = 0; i < prog_info.nr_map_ids; i++)
	{
		fd = bpf_map_get_fd_by_id(map_ids[i]);
		if (fd < 0)
			continue;

		memset(&map_info, 0, map_len);
		err = bpf_obj_get_info_by_fd(fd, &map_info, &map_len);
		if (err)
		{
			close(fd);
			continue;
		}

		if (!strncmp(map_info.name, "xsks_map", sizeof(map_info.name)) &&
			map_info.key_size == 4 && map_info.value_size == 4)
		{
			xsks_map_fd = fd;
			break;
		}

		close(fd);
	}

	free(map_ids);
	return xsks_map_fd;
}

static void enter_xsks_into_map_for_nic()
{
	int xsks_map;

	int first_nic_index = 0;

	xsks_map = lookup_bpf_map(xdp_program__fd(xdp_prog[first_nic_index]));
	if (xsks_map < 0)
	{
		fprintf(stderr, "ERROR: no xsks map found: %s\n",
				strerror(xsks_map));
		exit(EXIT_FAILURE);
	}


	printf("Update bpf map for xdp_prog[%d] %s, \n", first_nic_index, port_params[first_nic_index].iface);

	int y;
    for (y = 0; y < n_nic_ports; y++)
	{
		int fd = xsk_socket__fd(ports[y]->xsk);
		int key, ret;
		key = y;
		ret = bpf_map_update_elem(xsks_map, &key, &fd, 0);
		if (ret)
		{
			fprintf(stderr, "ERROR: bpf_map_update_elem %d %d\n", y, ret);
			exit(EXIT_FAILURE);
		}
	}
}

static void enter_xsks_into_map(u32 index)
{
	int i, xsks_map;

	xsks_map = lookup_bpf_map(xdp_program__fd(xdp_prog[index]));
	if (xsks_map < 0)
	{
		fprintf(stderr, "ERROR: no xsks map found: %s\n",
				strerror(xsks_map));
		exit(EXIT_FAILURE);
	}

	printf("Update bpf map for xdp_prog[%d] %s, \n", index, port_params[index].iface);

	int fd = xsk_socket__fd(ports[index]->xsk);
	int key, ret;
	i = 0;
	key = i;
	ret = bpf_map_update_elem(xsks_map, &key, &fd, 0);
	if (ret)
	{
		fprintf(stderr, "ERROR: bpf_map_update_elem %d %d\n", i, ret);
		exit(EXIT_FAILURE);
	}
}
