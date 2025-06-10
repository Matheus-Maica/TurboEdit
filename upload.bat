REM update version in pyproject.toml

rmdir -r .\dist\
rmdir -r .\turboedit.egg-info

python -m build
twine upload dist/*