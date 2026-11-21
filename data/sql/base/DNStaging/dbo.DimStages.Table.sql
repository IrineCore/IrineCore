USE [dnstaging]
GO
/****** Object:  Table [dbo].[DimStages]    Script Date: 11/14/2026 9:00:07 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[DimStages](
	[StageID] [int] NOT NULL,
	[StageName] [nvarchar](50) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[RecommendedCharacterLevel] [tinyint] NULL,
	[NestFlag] [bit] NULL,
 CONSTRAINT [PK_DimStages] PRIMARY KEY CLUSTERED 
(
	[StageID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]
GO
