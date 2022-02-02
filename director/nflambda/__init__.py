from .microarchitecture import *
from .platform import NFlambda
from .command import *
from .get_log import get_nflambda_log
from time import sleep
from plot import *
from em import *
from helper import *
from nf_registry import *
from .cat import *
import warnings

warnings.simplefilter(action='ignore', category=pd.errors.PerformanceWarning)

NF_FWD = 0
NF_GENERIC_DECOMPOSED = 1
NF_GENERIC_DECOMPOSED_PARALLEL = 2
NF_SEND = 100
NF_DROP = 101
NF_FREE_MESSAGE = 102