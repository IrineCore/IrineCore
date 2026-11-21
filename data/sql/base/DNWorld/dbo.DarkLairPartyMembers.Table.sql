USE [dnworld]
GO
/****** Object:  Table [dbo].[DarkLairPartyMembers]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[DarkLairPartyMembers](
	[DarkLairRankingID] [bigint] NOT NULL,
	[CharacterID] [bigint] NOT NULL,
	[CharacterLevel] [tinyint] NOT NULL,
	[JobCode] [tinyint] NOT NULL,
	[CompleteFlag] [bit] NOT NULL,
 CONSTRAINT [PK_DarkLairPartyMembers] PRIMARY KEY CLUSTERED 
(
	[DarkLairRankingID] ASC,
	[CharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
/****** Object:  Index [IX_NN_DarkLairPartyMembers_CharacterID]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_DarkLairPartyMembers_CharacterID] ON [dbo].[DarkLairPartyMembers]
(
	[CharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
GO
ALTER TABLE [dbo].[DarkLairPartyMembers]  WITH CHECK ADD  CONSTRAINT [FK_DarkLairRanking_DarkLairPartyMembers_DarkLairRankingID] FOREIGN KEY([DarkLairRankingID])
REFERENCES [dbo].[DarkLairRanking] ([DarkLairRankingID])
GO
ALTER TABLE [dbo].[DarkLairPartyMembers] CHECK CONSTRAINT [FK_DarkLairRanking_DarkLairPartyMembers_DarkLairRankingID]
GO
