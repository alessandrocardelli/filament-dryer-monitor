import json, os, re, subprocess
from pathlib import Path
def git(*a): return subprocess.check_output(['git',*a],text=True).strip()
def refs(): return {r:s for s,r in (x.split() for x in git('ls-remote','origin').splitlines())}
m=json.loads(Path('docs/evidence/repository/2026-10-29-refs.json').read_text())
main='bbee1b5ebf9fb8e4692d152cba169334299727a6'
exp='470f1e5181c9b327292af6a581c0bb407b25591d'
expname=m['experimental_branch']
temp='cleanup/consolidation-2026-10-29'
head=os.environ['GITHUB_SHA']
remote=refs()
assert remote['refs/heads/main']==main, 'Main moved; stop and inspect'
assert remote['refs/heads/'+expname]==exp, 'Experiment moved; stop and inspect'
assert remote['refs/heads/'+temp]==head, 'Cleanup branch moved'
for r in m['branches']:
    assert remote['refs/tags/'+r['tag']]==r['sha'], 'Missing/mismatched archive '+r['branch']
    if not r['retain']:
        assert remote['refs/heads/'+r['branch']]==r['sha'], 'Candidate moved '+r['branch']
git('fetch','--tags','origin')
git('diff','--exit-code','a60804d2aadc9cc5c2d0c291c59c54fee0d2c072',main,'--','hardware','docs/DECISIONS.md','firmware/AppConfig.h','firmware/SafetyController.h','firmware/firmware.ino')
git('diff','--exit-code','05d4c36d2378be9b09664545f37243a5dfa9f477',exp,'--','firmware',':!firmware/tests')
for ref in [main,exp]:
    files=git('ls-tree','-r','--name-only',ref).splitlines()
    for path in files:
        if not path.endswith('.md'):continue
        text=git('show',ref+':'+path)
        for url in re.findall(r'https?://[^\s)<>\x60]+',text):
            for row in m['branches']:
                if not row['retain']:
                    assert row['branch'] not in url, 'Unstable document URL: '+path+' '+url
# Preserve the exact one-time operation itself before deleting its temporary branch.
tag='refs/tags/archive/2026-10-29/consolidation-execution'
if tag in remote:assert remote[tag]==head
else:git('push','origin',head+':'+tag)
assert refs()[tag]==head
candidates=[(r['branch'],r['sha']) for r in m['branches'] if not r['retain']]
candidates.append((temp,head))
args=['push','--atomic','origin']
args+=['--force-with-lease=refs/heads/'+name+':'+sha for name,sha in candidates]
args+=[':refs/heads/'+name for name,sha in candidates]
print(git(*args))
remote=refs()
heads={r.removeprefix('refs/heads/'):s for r,s in remote.items() if r.startswith('refs/heads/')}
assert heads=={'main':main,expname:exp}, 'Unexpected final branch inventory'
for r in m['branches']:assert remote['refs/tags/'+r['tag']]==r['sha']
print('PASS: 20 original branches plus temporary cleanup branch removed atomically with exact-head leases.')
print('PASS: exactly main and STEP10A remain. All original heads and this execution preserved by remote tags.')
Path('consolidation-result.json').write_text(json.dumps({'branches':heads,'removed':[n for n,s in candidates],'original_archive_tags':22,'execution_tag':tag},indent=2)+'\n')
