# Copyright 1999-2007 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: $

inherit eutils python toolchain-funcs distutils

DESCRIPTION="DrQueue is an Open Source render farm managing software. It distributes shell based tasks such as rendering images on a per frame basis."
HOMEPAGE="http://www.drqueue.org/"
SRC_URI="http://drqueue.org/files/1-Sources_all_platforms/${PN}.${PV}.tgz"

LICENSE="GPL-2"
SLOT="0"
# ~mips keyword not possible because setuptools does not exist for mips
KEYWORDS="~x86 ~amd64 ~ppc"
IUSE="X python ruby"

RDEPEND="X? ( >=x11-libs/gtk+-2 )
	 python? ( dev-lang/python )
	 ruby? ( dev-lang/ruby )
	 app-shells/tcsh"

DEPEND="${RDEPEND}
	python? ( dev-lang/swig )
	ruby? ( dev-lang/swig )
	python? ( >=dev-python/setuptools-0.6_rc6 )
	>=dev-util/scons-0.97"

pkg_setup() {
	enewgroup drqueue
	enewuser drqueue -1 /bin/tcsh /dev/null daemon,drqueue
}

src_unpack() {
	unpack ${A}
	epatch "${FILESDIR}"/drqueue-0.64.3_mips_linux.patch
	if use ruby; then
		epatch "${FILESDIR}"/SConstruct.patch
	fi
}

src_compile() {
	if use X; then
		scons ${MAKEOPTS} build_drman=yes || die "scons failed"
	else
		scons ${MAKEOPTS} build_drqman=no || die "scons failed"
	fi
	if use python; then
		echo "compiling python bindings"
		cd ${S}/python/
		distutils_src_compile
	fi
	if use ruby; then
		echo "compiling ruby bindings"
		cd ${S}/ruby/
		ruby extconf.rb
		emake
	fi
}

pkg_preinst() {
	# stop daemons since script is being updated
	[ -n "$(pidof drqsd)" -a -x /etc/init.d/drqsd ] && \
			/etc/init.d/drqsd stop
	[ -n "$(pidof drqmd)" -a -x /etc/init.d/drqmd ] && \
			/etc/init.d/drqmd stop
}

src_install() {

	dodir /usr/share
	scons -j1 PREFIX=${D}/usr/share install || die "install failed"

	# install conf files
	insinto /etc/conf.d
	newins "${FILESDIR}/${PATCHLEVEL}/conf-drqsd-0.64.3" drqsd
	newins "${FILESDIR}/${PATCHLEVEL}/conf-drqmd-0.64.3" drqmd

	# install env file 
	insinto /etc/env.d
	newins "${FILESDIR}/${PATCHLEVEL}/env" 02drqueue

	# install init scripts
	exeinto /etc/init.d
	newexe "${FILESDIR}/${PATCHLEVEL}/init-drqsd-0.64.3" drqsd
	newexe "${FILESDIR}/${PATCHLEVEL}/init-drqmd-0.64.3" drqmd

	# create the drqueue pid directory
	dodir /var/run/drqueue
	keepdir /var/run/drqueue

	# all etcs go to /etc/drqueue
	dodir /etc/drqueue
	mv ${D}/usr/share/drqueue/etc/* ${D}/etc/drqueue
	rmdir ${D}/usr/share/drqueue/etc

	# all bins go to /usr/bin
	dodir /usr/bin
	mv ${D}/usr/share/drqueue/bin/* ${D}/usr/bin
	rmdir ${D}/usr/share/drqueue/bin

	# TODO: where should that go?
	rm -rf ${D}/usr/bin/viewcmd

	if use python; then
		cd ${S}/python/
		distutils_src_install
		dodir /usr/share/${PN}/python

		# Install DRKeewee web service and example python scripts
		cp -R DrKeewee ${D}/usr/share/${PN}/python
		cp -R examples ${D}/usr/share/${PN}/python
	fi
	if use ruby; then
		cd ${S}/ruby/
		make DESTDIR=${D} install
	fi
}


pkg_postinst() {
	einfo "Edit /etc/conf.d/drqsd /etc/env.d/02drqueue"
	einfo "and /etc/conf.d/drqmd DRQUEUE_MASTER=\"hostname\""
	einfo "to reflect your master's hostname."
	einfo ""
	einfo "/etc/drqueue contains further files"
	einfo "which require configuration, mainly"
	einfo "master.conf and slave.conf"
	einfo "DrKeewee is installed with the python"
	einfo "USE flag and can be found in /usr/share/drqueue/python"
}


