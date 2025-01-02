import typing

import model
from writer import Writer

WORLD_VERSION_ALL = model.WorldVersion(major=0xFF, minor=0xFF, patch=0xFF, build=0xFFFF)
VANILLA = model.WorldVersion(major=1, minor=12, patch=1, build=5875)
TBC = model.WorldVersion(major=2, minor=4, patch=3, build=8606)
WRATH = model.WorldVersion(major=3, minor=3, patch=5, build=12340)

VERSIONS = [VANILLA, TBC, WRATH]

LOGIN_VERSION_ALL = 0xFFFF

IS_CPP = False
def is_cpp() -> bool:
    global IS_CPP
    return IS_CPP

def set_cpp(cpp: bool):
    global IS_CPP
    IS_CPP = cpp

def write_file_if_not_same(s: Writer, path: str):
    overwrite = False
    try:
        f = open(path, "r")
        data = f.read()
        f.close()
        if data != s.inner():
            overwrite = True
    except:
        overwrite = True

    if overwrite:
        print(f"Writing {path}")
        with open(path, "w") as f:
            f.write(s.inner())


def should_print_container(container: model.Container, v: model.WorldVersion) -> bool:
    if not world_version_matches(container.tags, v):
        return False

    return True


def world_version_satisfies(t: model.WorldVersion, o: model.WorldVersion) -> bool:
    """
    Version `t` is fully satisfied by version `o`.
    """
    if t.major != o.major:
        return False

    # Major versions are equal

    def test(t: typing.Optional[int], o: typing.Optional[int]):
        if t is None and o is None:
            return True
        # Either t or o are not None

        # t is more general than o
        if t is None and o is not None:
            return False

        # t is less general than o
        if t is not None and o is None:
            return True

        if t != o:
            return False

        return None

    minor = test(t.minor, o.minor)
    if minor is not None:
        return minor

    patch = test(t.patch, o.patch)
    if patch is not None:
        return patch

    build = test(t.build, o.build)
    if build is not None:
        return build

    return True


def world_version_is_wrath(version: model.WorldVersion) -> bool:
    return version == WRATH


def world_version_is_tbc(version: model.WorldVersion) -> bool:
    return version == TBC


def world_version_is_vanilla(version: model.WorldVersion) -> bool:
    return version == VANILLA


def world_version_is_all(version: model.WorldVersion) -> bool:
    return version == WORLD_VERSION_ALL


def world_version_matches(tags: model.ObjectTags, version: model.WorldVersion) -> bool:
    match tags.version:
        case model.ObjectVersionsLogin(version_type=version_type):
            raise Exception("invalid version")
        case model.ObjectVersionsWorld(version_type=version_type):
            match version_type:
                case model.WorldVersionsAll():
                    return True
                case model.WorldVersionsSpecific(versions=versions):
                    for v in versions:
                        if world_version_satisfies(version, v):
                            return True
                case _:
                    raise Exception("invalid world versions type")

    return False


def container_is_unencrypted(name: str) -> bool:
    return name == "CMSG_AUTH_SESSION" or name == "SMSG_AUTH_CHALLENGE"


def version_to_module_name(v: typing.Union[int | model.WorldVersion]) -> str:
    if type(v) is int:
        return login_version_to_module_name(v)
    elif type(v) is model.WorldVersion:
        return world_version_to_module_name(v)
    else:
        raise Exception("invalid version type")


def world_version_to_module_name(v: model.WorldVersion) -> str:
    if v == WORLD_VERSION_ALL:
        return "all"

    match v:
        # Hack around not having full alpha support
        case model.WorldVersion(major=0):
            return "vanilla"
        case model.WorldVersion(major=1):
            return "vanilla"
        case model.WorldVersion(major=2):
            return "tbc"
        case model.WorldVersion(major=3):
            return "wrath"
        case v:
            raise Exception(f"unknown version {v}")


def world_version_to_title_name(v: model.WorldVersion) -> str:
    return world_version_to_module_name(v).capitalize()


def login_version_to_module_name(v: int) -> str:
    if v == LOGIN_VERSION_ALL:
        return "all"
    else:
        return f"version{v}"


def library_type(tags: model.ObjectTags) -> str:
    if is_world(tags):
        return "world"
    else:
        return "login"

def snake_case_to_pascal_case(s: str) -> str:
    new: str = ""
    next_upper = True

    for i in s:
        if next_upper:
            new += i.upper()
            next_upper = False
            continue

        if i == '_':
            next_upper = True
            continue

        new += i

    return new

def pascal_case_to_snake_case(s: str) -> str:
    new = ""
    first_char = True

    for i in s:
        if i == '_':
            first_char = False
            continue
        if i.isupper() and not first_char:
            new += f"_{i.lower()}"
        else:
            new += i

        first_char = False

    return new


def first_login_version(tags: model.ObjectTags) -> int:
    match tags.version:
        case model.ObjectVersionsLogin(version_type=version_type):
            match version_type:
                case model.LoginVersionsAll():
                    return LOGIN_VERSION_ALL
                case model.LoginVersionsSpecific(versions=versions):
                    return versions[0]
                case _:
                    raise Exception("invalid login versions type")
        case _:
            raise Exception("invalid version")


def first_version_as_module(tags: model.ObjectTags) -> str:
    match tags.version:
        case model.ObjectVersionsLogin(version_type=l):
            match l:
                case model.LoginVersionsAll():
                    return login_version_to_module_name(LOGIN_VERSION_ALL)
                case model.LoginVersionsSpecific(versions=versions):
                    return login_version_to_module_name(versions[0])
                case _:
                    raise Exception("invalid login versions type")
        case model.ObjectVersionsWorld(version_type=w):
            match w:
                case model.WorldVersionsAll():
                    return "all"
                case model.WorldVersionsSpecific(versions=world_versions):
                    for v in world_versions:
                        return world_version_to_module_name(v)

                case _:
                    raise Exception("invalid world versions type")

    raise Exception("invalid version type")


def get_export_define(tags: model.ObjectTags) -> str:
    if is_cpp():
        return "WOW_WORLD_MESSAGES_CPP_EXPORT" if is_world(tags) else "WOW_LOGIN_MESSAGES_CPP_EXPORT"
    else:
        return "WOW_WORLD_MESSAGES_C_EXPORT" if is_world(tags) else "WOW_LOGIN_MESSAGES_C_EXPORT"


def get_type_prefix(tags: model.ObjectTags) -> str:
    return "WowWorld" if is_world(tags) else "WowLogin"


def is_world(tags: model.ObjectTags) -> bool:
    match tags.version:
        case model.ObjectVersionsLogin():
            return False
        case model.ObjectVersionsWorld():
            return True
        case v:
            raise Exception(f"unknown tag {v}")


def version_matches(tags: model.ObjectTags, value: typing.Union[int | model.WorldVersion]) -> bool:
    if type(value) is int:
        return login_version_matches(tags, value)
    elif type(value) is model.WorldVersion:
        return world_version_matches(tags, value)
    else:
        raise Exception(f"unknown tag {value}")


def login_version_matches(tags: model.ObjectTags, value: int) -> bool:
    match tags.version:
        case model.ObjectVersionsLogin(version_type=version_type):
            match version_type:
                case model.LoginVersionsAll():
                    return True
                case model.LoginVersionsSpecific(versions=versions):
                    return value in versions
                case _:
                    raise Exception("invalid login versions type")
        case _:
            raise Exception("invalid version")


def container_needs_size_in_read(container: model.Container) -> bool:
    def inner_if(statement: model.IfStatement) -> bool:
        for m in statement.members:
            if inner(m):
                return True

        for elseif in statement.else_if_statements:
            if inner_if(elseif):
                return True

        return False

    def inner(m: model.StructMember) -> bool:
        match m:
            case model.StructMemberDefinition(struct_member_content=d):
                match d.data_type:
                    case model.DataTypeArray(compressed=compressed, size=size):
                        if compressed:
                            return True

                        match size:
                            case model.ArraySizeEndless():
                                return True

            case model.StructMemberIfStatement(struct_member_content=statement):
                if inner_if(statement):
                    return True

        return False

    if container.tags.compressed:
        return True

    if container.optional is not None:
        return True

    for m in container.members:
        if inner(m):
            return True

    return False
