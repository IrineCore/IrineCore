USE [dngsm]
GO
/****** Object:  Table [dbo].[ExtGuild]    Script Date: 11/14/2026 8:47:03 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[ExtGuild](
	[id] [int] NULL,
	[_GuildLevel] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_ReqGold] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_ReqGuildPoint] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_ReqGuildUser] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_CanSkill] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_CanStorage] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_CanGvG] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_CanShop] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_CanMark] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_CanHouse] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_CanFarm] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_GuildUserMax] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_Gstorage] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[PartitionKey] [nvarchar](255) COLLATE SQL_Latin1_General_CP1_CI_AS NULL
) ON [PRIMARY]
GO
/****** Object:  Index [IX_UN_ExtGuild_id]    Script Date: 11/14/2026 8:47:09 AM ******/
CREATE UNIQUE NONCLUSTERED INDEX [IX_UN_ExtGuild_id] ON [dbo].[ExtGuild]
(
	[id] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = ON, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
