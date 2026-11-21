USE [dnworld]
GO
/****** Object:  Table [dbo].[GuildDepots]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[GuildDepots](
	[GuildID] [int] NOT NULL,
	[ItemSerial] [bigint] NOT NULL,
	[LockFlag] [bit] NOT NULL,
	[LockDate] [datetime2](0) NULL,
	[MaterializedItemID] [bigint] NOT NULL,
 CONSTRAINT [PK_GuildDepots] PRIMARY KEY CLUSTERED 
(
	[GuildID] ASC,
	[ItemSerial] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
/****** Object:  Index [IX_NN_GuildDepots_MaterializedItemID]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_GuildDepots_MaterializedItemID] ON [dbo].[GuildDepots]
(
	[MaterializedItemID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
/****** Object:  Index [IX_UN_GuildDepots_ItemSerial]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE UNIQUE NONCLUSTERED INDEX [IX_UN_GuildDepots_ItemSerial] ON [dbo].[GuildDepots]
(
	[ItemSerial] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
ALTER TABLE [dbo].[GuildDepots]  WITH CHECK ADD  CONSTRAINT [FK_Guilds_GuildDepots_GuildID] FOREIGN KEY([GuildID])
REFERENCES [dbo].[Guilds] ([GuildID])
GO
ALTER TABLE [dbo].[GuildDepots] CHECK CONSTRAINT [FK_Guilds_GuildDepots_GuildID]
GO
