USE [dngsm]
GO
/****** Object:  Table [dbo].[MailAttachItems]    Script Date: 11/14/2026 8:47:03 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[MailAttachItems](
	[AuditLogID] [int] NOT NULL,
	[MailAttachItemID] [int] IDENTITY(1,1) NOT NULL,
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
 CONSTRAINT [PK_MailAttachItems] PRIMARY KEY CLUSTERED 
(
	[AuditLogID] ASC,
	[MailAttachItemID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]
GO
ALTER TABLE [dbo].[MailAttachItems]  WITH CHECK ADD  CONSTRAINT [FK_SendMailLog_MailAttachItems_AuditLogID] FOREIGN KEY([AuditLogID])
REFERENCES [dbo].[SendMailLog] ([AuditLogID])
GO
ALTER TABLE [dbo].[MailAttachItems] CHECK CONSTRAINT [FK_SendMailLog_MailAttachItems_AuditLogID]
GO
