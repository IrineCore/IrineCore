USE [dnworld]
GO
/****** Object:  Table [dbo].[DarkLairRanking]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[DarkLairRanking](
	[DarkLairRankingID] [bigint] IDENTITY(1,1) NOT NULL,
	[MapID] [int] NOT NULL,
	[PartyUserCount] [tinyint] NOT NULL,
	[PlayRound] [smallint] NOT NULL,
	[PlayTime] [int] NOT NULL,
	[PartyName] [nvarchar](50) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[RegisterDate] [datetime] NOT NULL,
	[BestFlag] [bit] NOT NULL,
	[BlockFlag] [int] NOT NULL,
 CONSTRAINT [PK_DarkLairRanking] PRIMARY KEY CLUSTERED 
(
	[DarkLairRankingID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
/****** Object:  Index [IX_NN_DarkLairRanking_MapID]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_DarkLairRanking_MapID] ON [dbo].[DarkLairRanking]
(
	[MapID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
GO
/****** Object:  Index [IX_NNIF_DarkLairRanking_MapID_PartyUserCount_PlayRound_PlayTime]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE NONCLUSTERED INDEX [IX_NNIF_DarkLairRanking_MapID_PartyUserCount_PlayRound_PlayTime] ON [dbo].[DarkLairRanking]
(
	[MapID] ASC,
	[PartyUserCount] ASC,
	[PlayRound] DESC,
	[PlayTime] ASC
)
INCLUDE([BlockFlag]) 
WHERE ([BestFlag]=CONVERT([bit],(1),(0)))
WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
GO
ALTER TABLE [dbo].[DarkLairRanking] ADD  CONSTRAINT [DF_DarkLairRanking_BestFlag]  DEFAULT ((0)) FOR [BestFlag]
GO
ALTER TABLE [dbo].[DarkLairRanking] ADD  CONSTRAINT [DF_DarkLairRanking_BlockFlag]  DEFAULT ((0)) FOR [BlockFlag]
GO
