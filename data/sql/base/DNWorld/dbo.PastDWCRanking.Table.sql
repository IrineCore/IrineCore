USE [dnworld]
GO
/****** Object:  Table [dbo].[PastDWCRanking]    Script Date: 11/14/2026 8:54:16 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[PastDWCRanking](
	[DWCSequence] [int] NOT NULL,
	[Ranking] [int] NOT NULL,
	[DWCTeamID] [int] NOT NULL,
	[DWCTeamName] [nvarchar](50) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[Win] [int] NULL,
	[Lose] [int] NULL,
	[Draw] [int] NULL,
	[DWCPoint] [int] NULL,
	[ChangedRanking] [int] NOT NULL,
	[DWCTeamLeaderCharacterInfo] [nvarchar](80) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[DWCTeamMemberCharacterInfo] [nvarchar](160) COLLATE Chinese_PRC_CI_AS NULL,
 CONSTRAINT [PK_PastDWCRanking] PRIMARY KEY CLUSTERED 
(
	[DWCSequence] ASC,
	[Ranking] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
