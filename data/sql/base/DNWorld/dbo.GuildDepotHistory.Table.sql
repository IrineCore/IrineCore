USE [dnworld]
GO
/****** Object:  Table [dbo].[GuildDepotHistory]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[GuildDepotHistory](
	[GuildID] [int] NOT NULL,
	[GuildDepotHistoryID] [bigint] IDENTITY(1,1) NOT NULL,
	[CharacterID] [int] NOT NULL,
	[CharacterName] [nvarchar](50) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[ItemCoinFlag] [bit] NOT NULL,
	[InOutFlag] [bit] NOT NULL,
	[ItemID] [int] NULL,
	[ItemLevel] [tinyint] NULL,
	[ItemCount] [smallint] NULL,
	[GuildCoin] [int] NULL,
	[RegisterDate] [datetime2](0) NOT NULL,
	[GuildCoinAfter] [bigint] NULL,
 CONSTRAINT [PK_GuildDepotHistory] PRIMARY KEY CLUSTERED 
(
	[GuildID] ASC,
	[GuildDepotHistoryID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
/****** Object:  Index [IX_NN_GuildDepotHistory_RegisterDate]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_GuildDepotHistory_RegisterDate] ON [dbo].[GuildDepotHistory]
(
	[RegisterDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
ALTER TABLE [dbo].[GuildDepotHistory]  WITH CHECK ADD  CONSTRAINT [FK_Guilds_GuildDepotHistory_GuildID] FOREIGN KEY([GuildID])
REFERENCES [dbo].[Guilds] ([GuildID])
GO
ALTER TABLE [dbo].[GuildDepotHistory] CHECK CONSTRAINT [FK_Guilds_GuildDepotHistory_GuildID]
GO
