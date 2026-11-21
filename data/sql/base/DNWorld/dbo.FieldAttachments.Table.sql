USE [dnworld]
GO
/****** Object:  Table [dbo].[FieldAttachments]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[FieldAttachments](
	[FieldAttachmentID] [int] IDENTITY(1,1) NOT NULL,
	[FarmID] [int] NOT NULL,
	[FieldIndex] [smallint] NOT NULL,
	[AttachedItemID] [int] NOT NULL,
	[AttachedDate] [datetime2](0) NOT NULL,
 CONSTRAINT [PK_FieldAttachments] PRIMARY KEY CLUSTERED 
(
	[FieldAttachmentID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
/****** Object:  Index [IX_NN_FieldAttachments_FarmID_FieldIndex]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_FieldAttachments_FarmID_FieldIndex] ON [dbo].[FieldAttachments]
(
	[FarmID] ASC,
	[FieldIndex] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
ALTER TABLE [dbo].[FieldAttachments] ADD  CONSTRAINT [DF_FieldAttachments_AttachedDate]  DEFAULT (getdate()) FOR [AttachedDate]
GO
ALTER TABLE [dbo].[FieldAttachments]  WITH CHECK ADD  CONSTRAINT [FK_Fields_FieldAttachments_FarmID_FieldIndex] FOREIGN KEY([FarmID], [FieldIndex])
REFERENCES [dbo].[Fields] ([FarmID], [FieldIndex])
GO
ALTER TABLE [dbo].[FieldAttachments] CHECK CONSTRAINT [FK_Fields_FieldAttachments_FarmID_FieldIndex]
GO
