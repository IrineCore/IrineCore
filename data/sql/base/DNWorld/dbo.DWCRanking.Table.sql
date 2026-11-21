USE [dnworld]
GO
/****** Object:  Table [dbo].[DWCRanking]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[DWCRanking](
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
 CONSTRAINT [PK_DWCRanking] PRIMARY KEY CLUSTERED 
(
	[Ranking] ASC,
	[DWCTeamID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
/****** Object:  Index [IX_UN_DWCRanking_DWCTeamID]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE UNIQUE NONCLUSTERED INDEX [IX_UN_DWCRanking_DWCTeamID] ON [dbo].[DWCRanking]
(
	[DWCTeamID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
GO
