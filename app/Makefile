# name of your application
APPLICATION = dust

# If no BOARD is found in the environment, use this default:
BOARD ?= iotlab-m3

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/..

# Set this to 1 to enable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 1

DISABLE_MODULE += auto_init_at86rf2xx

USEMODULE += od
USEMODULE += shell
USEMODULE += shell_commands
USEMODULE += ps
USEMODULE += xtimer

# USEMODULE += auto_init_gnrc_netif

USEMODULE += at86rf2xx
USEMODULE += at86rf231
USEMODULE += lpsxxx
USEMODULE += lps331ap


include $(RIOTBASE)/Makefile.include
