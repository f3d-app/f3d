import useDocusaurusContext from '@docusaurus/useDocusaurusContext';
import Layout from '@theme/Layout';
import HomepageFeatures from '@site/src/components/HomepageFeatures';

import styles from './index.module.css'
import Link from '@docusaurus/Link';
import { GalleryCarousel } from '@site/src/components/GalleryCarousel';
import { useEffect, useState } from 'react';
import { guess_client_OS } from '@site/src/lib/utils';

function HomepageHeader() {
  const [OS, setOS] = useState<null|string>(null)

  useEffect(() => {
    setOS(guess_client_OS())
  }, [])

  const {siteConfig} = useDocusaurusContext();
  return (
    <header className={styles.header}>
      <div className={styles.container}>
        <h1 className={styles.heading}>
          Fast and minimalist 3D viewer
        </h1>
        <p className={styles.description}>
          F3D is designed to support multiple file formats, from digital content to scientific datasets, handle animations, support many rendering and texturing options and real time physically based rendering and raytracing.
        </p>
      </div>
      <div className={styles.flexContainer}>
        <div>
          <Link className={styles.button} data-variant="outline" to={'docs/HOME'}>Docs</Link>
        </div>
        <div>
          <a className={styles.button} data-variant="primary">{`Download for ${OS} (64bit)`}</a>
        </div>
      </div>
    </header>
  );
}

export default function Home(): JSX.Element {
  const {siteConfig} = useDocusaurusContext();
  return (
    <Layout
      title={`F3D docs`}
      description="F3D is a fast minimal 3d viewer">
      <HomepageHeader />
      <div className="mx-auto my-4">
        <GalleryCarousel />
      </div>
      <div className={styles.subSection}>
        <h2 className={styles.subHeading}>Our contributors</h2>
        <div className={styles.contributions}>
          <a href="https://github.com/f3d-app/f3d/graphs/contributors">
            <img src="https://contrib.rocks/image?repo=f3d-app/f3d" />
          </a>
        </div>
      </div>
    </Layout>
  );
}
