USE [dnworld]
GO
/****** Object:  Table [dbo].[DarkLairPersonalBest]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[DarkLairPersonalBest](
	[CharacterID] [bigint] NOT NULL,
	[MapID] [int] NOT NULL,
	[PartyUserCount] [tinyint] NOT NULL,
	[DarkLairRankingID] [bigint] NOT NULL,
 CONSTRAINT [PK_DarkLairPersonalBest] PRIMARY KEY CLUSTERED 
(
	[CharacterID] ASC,
	[MapID] ASC,
	[PartyUserCount] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
/****** Object:  Index [IX_NN_DarkLairPersonalBest_DarkLairRankingID]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_DarkLairPersonalBest_DarkLairRankingID] ON [dbo].[DarkLairPersonalBest]
(
	[DarkLairRankingID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
GO
ALTER TABLE [dbo].[DarkLairPersonalBest]  WITH CHECK ADD  CONSTRAINT [FK_DarkLairRanking_DarkLairPersonalBest_DarkLairRankingID] FOREIGN KEY([DarkLairRankingID])
REFERENCES [dbo].[DarkLairRanking] ([DarkLairRankingID])
GO
ALTER TABLE [dbo].[DarkLairPersonalBest] CHECK CONSTRAINT [FK_DarkLairRanking_DarkLairPersonalBest_DarkLairRankingID]
GO
