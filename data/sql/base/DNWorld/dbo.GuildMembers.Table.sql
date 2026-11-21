USE [dnworld]
GO
/****** Object:  Table [dbo].[GuildMembers]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[GuildMembers](
	[GuildID] [int] NOT NULL,
	[CharacterID] [bigint] NOT NULL,
	[GuildMemberRoleCode] [tinyint] NOT NULL,
	[SelfIntroduction] [nvarchar](300) COLLATE Chinese_PRC_CI_AS NULL,
	[RegisterDate] [datetime2](0) NOT NULL,
	[LastGuildDepotUseDate] [smalldatetime] NULL,
	[DailyTakeItemCount] [int] NOT NULL,
	[DailyTakeCoinAmount] [int] NOT NULL,
	[TotalGuildPoint] [int] NOT NULL,
	[UsedGuildPoint] [int] NOT NULL,
	[CharacterJoinLevel] [tinyint] NULL,
	[GuildContributionPoint] [int] NOT NULL,
 CONSTRAINT [PK_GuildMembers] PRIMARY KEY CLUSTERED 
(
	[GuildID] ASC,
	[CharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
/****** Object:  Index [IX_UN_GuildMembers_CharacterID]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE UNIQUE NONCLUSTERED INDEX [IX_UN_GuildMembers_CharacterID] ON [dbo].[GuildMembers]
(
	[CharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
ALTER TABLE [dbo].[GuildMembers] ADD  CONSTRAINT [DF_GuildMembers_RegisterDate]  DEFAULT (getdate()) FOR [RegisterDate]
GO
ALTER TABLE [dbo].[GuildMembers] ADD  CONSTRAINT [DF_GuildMembers_DailyTakeItemCount]  DEFAULT ((0)) FOR [DailyTakeItemCount]
GO
ALTER TABLE [dbo].[GuildMembers] ADD  CONSTRAINT [DF_GuildMembers_DailyTakeCoinAmount]  DEFAULT ((0)) FOR [DailyTakeCoinAmount]
GO
ALTER TABLE [dbo].[GuildMembers] ADD  CONSTRAINT [DF_GuildMembers_TotalGuildPoint]  DEFAULT ((0)) FOR [TotalGuildPoint]
GO
ALTER TABLE [dbo].[GuildMembers] ADD  CONSTRAINT [DF_GuildMembers_UsedGuildPoint]  DEFAULT ((0)) FOR [UsedGuildPoint]
GO
ALTER TABLE [dbo].[GuildMembers] ADD  CONSTRAINT [DF_GuildMembers_GuildContributionPoint]  DEFAULT ((0)) FOR [GuildContributionPoint]
GO
ALTER TABLE [dbo].[GuildMembers]  WITH CHECK ADD  CONSTRAINT [FK_Guilds_GuildMembers_GuildID] FOREIGN KEY([GuildID])
REFERENCES [dbo].[Guilds] ([GuildID])
GO
ALTER TABLE [dbo].[GuildMembers] CHECK CONSTRAINT [FK_Guilds_GuildMembers_GuildID]
GO
