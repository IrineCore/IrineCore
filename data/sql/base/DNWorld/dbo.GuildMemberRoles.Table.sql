USE [dnworld]
GO
/****** Object:  Table [dbo].[GuildMemberRoles]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[GuildMemberRoles](
	[GuildID] [int] NOT NULL,
	[GuildMemberRoleCode] [tinyint] NOT NULL,
	[DailyTakeItemLimit] [int] NOT NULL,
	[DailyTakeCoinLimit] [int] NOT NULL,
	[Privilege] [int] NOT NULL,
 CONSTRAINT [PK_GuildMemberRoles] PRIMARY KEY CLUSTERED 
(
	[GuildID] ASC,
	[GuildMemberRoleCode] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
ALTER TABLE [dbo].[GuildMemberRoles]  WITH CHECK ADD  CONSTRAINT [FK_Guilds_GuildMemberRoles_GuildID] FOREIGN KEY([GuildID])
REFERENCES [dbo].[Guilds] ([GuildID])
GO
ALTER TABLE [dbo].[GuildMemberRoles] CHECK CONSTRAINT [FK_Guilds_GuildMemberRoles_GuildID]
GO
