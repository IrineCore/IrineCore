USE [dnworld]
GO
/****** Object:  Table [dbo].[AttachItemsForMailSchedule]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[AttachItemsForMailSchedule](
	[MailScheduleID] [int] NOT NULL,
	[AttachItemIDForMailSchedule] [int] IDENTITY(1,1) NOT NULL,
	[ItemID] [int] NOT NULL,
	[ItemCount] [smallint] NOT NULL,
	[ItemDurability] [smallint] NOT NULL,
	[RandomSeed] [int] NOT NULL,
	[ItemLevel] [tinyint] NOT NULL,
	[ItemPotential] [tinyint] NOT NULL,
	[SoulBoundFlag] [bit] NOT NULL,
	[SealCount] [tinyint] NOT NULL,
	[ItemOption] [tinyint] NOT NULL,
	[EternityFlag] [bit] NOT NULL,
	[ItemLifespan] [int] NOT NULL,
	[DragonJewelType] [tinyint] NULL,
 CONSTRAINT [PK_AttachItemsForMailSchedule] PRIMARY KEY CLUSTERED 
(
	[MailScheduleID] ASC,
	[AttachItemIDForMailSchedule] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
ALTER TABLE [dbo].[AttachItemsForMailSchedule]  WITH CHECK ADD  CONSTRAINT [FK_MailSchedules_AttachItemsForMailToAll_MailScheduleID] FOREIGN KEY([MailScheduleID])
REFERENCES [dbo].[MailSchedules] ([MailScheduleID])
GO
ALTER TABLE [dbo].[AttachItemsForMailSchedule] CHECK CONSTRAINT [FK_MailSchedules_AttachItemsForMailToAll_MailScheduleID]
GO
