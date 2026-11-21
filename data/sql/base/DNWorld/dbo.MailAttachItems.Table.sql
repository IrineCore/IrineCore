USE [dnworld]
GO
/****** Object:  Table [dbo].[MailAttachItems]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[MailAttachItems](
	[MailID] [bigint] NOT NULL,
	[MailAttachItemID] [int] IDENTITY(1,1) NOT NULL,
	[ItemSerial] [bigint] NOT NULL,
	[ItemID] [int] NOT NULL,
	[FirstAttachItemFlag] [bit] NOT NULL,
	[ReceiveFlag] [bit] NOT NULL,
	[AttachItemCount] [smallint] NOT NULL,
 CONSTRAINT [PK_MailAttachItems] PRIMARY KEY CLUSTERED 
(
	[MailID] ASC,
	[MailAttachItemID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
ALTER TABLE [dbo].[MailAttachItems]  WITH CHECK ADD  CONSTRAINT [FK_Mails_MailAttachItems_MailID] FOREIGN KEY([MailID])
REFERENCES [dbo].[Mails] ([MailID])
GO
ALTER TABLE [dbo].[MailAttachItems] CHECK CONSTRAINT [FK_Mails_MailAttachItems_MailID]
GO
